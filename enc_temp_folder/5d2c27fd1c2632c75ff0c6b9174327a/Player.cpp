#include "pch.h"
#include "Player.h"
#include "../SketchMusic.h"
#include "../base/Cursor.h"
#include "../base/Symbol.h"
#include "../base/Text.h"
#include "ISoundEngine.h"
#include "SoundEnginePool.h"
#include "Instrument.h"
#include "agents.h"
#include "ppltasks.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

using namespace Windows::Foundation::Collections;

SketchMusic::Player::Player::Player()
{
	this->_cursor = ref new SketchMusic::Cursor;
	needMetronome = true;
	_BPM = 120;
	cycling = false;

	_BPM = 80;
	// do - чтобы можно было "скипнуть" инициализацию, если то-то не удалось
	do
	{
		// создаём объект XAudio2
		HRESULT hr = XAudio2Create(&pXAudio2);
		if (FAILED(hr))
		{
			//CurPos->Text = "XAudio2Create failure: " + hr.ToString();
			break;
		}

		// создаём мастеринговый голос
		hr = pXAudio2->CreateMasteringVoice(&pMasteringVoice);
		if (FAILED(hr))
		{
			//CurPos->Text = "CreateMasteringVoice failure: " + hr.ToString();
			break;
		}

		_engines = ref new SoundEnginePool(pXAudio2);
		_metronome = _engines->GetSoundEngine(ref new Instrument("Metronome"));
	} while (0);
}

/**
Метод проигрывает указанную ноту с продолжительностью по умолчанию.
Используется для проигрывания ноты при нажатии на клавиатуру
Возможно, стоит объединить с методом playNotes или сделать перегрузкой...
*/
void SketchMusic::Player::Player::playSingleNote(SketchMusic::INote^ note, SketchMusic::Instrument^ instrument, int duration, SketchMusic::Player::NoteOff^ noteOff)
{
	auto playNote = concurrency::create_task([=]
	{
		// если имеющийся не устраивает
		if (_keyboardEngine && (_keyboardEngine->getInstrument()->_name != instrument->_name))
		{
			_engines->ReleaseSoundEngine(_keyboardEngine);
			_keyboardEngine = nullptr;
		}

		if (_keyboardEngine == nullptr)
		{
			_keyboardEngine = _engines->GetSoundEngine(instrument);
		}

		if (_keyboardEngine)
		{
			_keyboardEngine->Play(note, 0, noteOff);
		}
	});
}


void SketchMusic::Player::Player::playMetronome()
{
	if (_metronome)
	{
		_metronome->Play(ref new SNote(17), 20, nullptr); // TODO : незачем каждый раз создавать новую ноту, можно хранить значение
	}
}

/**
Принимает текст и проигрывает его начиная со start либо до конца, либо пока не придёт
команда остановиться.
*/
void SketchMusic::Player::Player::playText(Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ texts, SketchMusic::Cursor^ pos)
{
	// - для каждой из дорожек инициализировать инструмент
	// - - создать по соурс войсу на каждый семпл
	// - - если такой инструмент уже проинициализирован, не трогать
	// - пройтись по всем нотам
	this->_state = SketchMusic::Player::S_PLAY;
	

	SketchMusic::Cursor^ cursor = ref new SketchMusic::Cursor(pos);
	//_cursor->moveTo(pos);

	// ----
	// Этап 1: собираем итераторы со всего набора текстов, которые указывают
	// на ноты, ближайшие к данному положению курсора.
	// Заодно инициализируем SoundEngine, отвечающие за инструменты, связанные с текстом, если таковые ещё не проинициализированы
	auto iterMap 
		= new std::vector< std::pair<SketchMusic::Player::ISoundEngine^, std::pair<IIterator < SketchMusic::PositionedSymbol^ > ^, IVector< SketchMusic::PositionedSymbol^ > ^> > >;

	auto getIterators = concurrency::create_task([texts, iterMap, this, cursor]
	{
		for (auto text : texts)
		{
			
			// получаем список символов в данном тексте
			auto symbols = text->getText();
			// находим ближайший к старту символ (ПОСЛЕ старта)
			
			// TODO : отдельно стоит отслеживать ближайший к старту (ПЕРЕД стартом)
			// символ начала строки - там будет храниться информация о темпе, гамме и т.д.
			auto iter = symbols->First();
			while (iter->HasCurrent)
			{
				if (iter->Current->_pos->LT(cursor))
				{
					iter->MoveNext();
				}
				else
				{
					// отыскиваем среди енжинов подходящий
					ISoundEngine^ engine = this->_engines->GetSoundEngine(text->instrument);
					
					iterMap->push_back(std::make_pair(engine, std::make_pair(iter, symbols)));
					break;
				}
			}
		}
	})
		.then([this, iterMap, cursor]
	{
		int tempState = SketchMusic::Player::S_PLAY;
		__time64_t ptime, ctime;
		_time64(&ptime);
		int pbeat = -1;
		
		clock_t pclock = clock();
		auto pClock = Clock::now();

		while (tempState == SketchMusic::Player::S_PLAY)
		{
			// запустить асинхронно
			//concurrency::parallel_for_each(iterList->begin(), iterList->end(), [this](Windows::Foundation::Collections::IIterator<SketchMusic::PositionedSymbol^>^ iter)
			// - один итератор = один текст = один инструмент = один саунденжин = несколько нот
			tempState = S_STOP;
			for (auto iter = iterMap->begin(); iter < iterMap->end(); iter++)
			{
				auto pIter = iter->second.first;
				if (pIter->HasCurrent)
				{
					tempState = this->_state;
					SketchMusic::Cursor^ pos = pIter->Current->_pos;
					
					// выбираем все ноты в данной точке
					// UPD : при переходе на флоат-версию курсора требуем, чтобы пос был меньше чем текущее положение,
					// чтобы избежать недоразумений. Уже прошедшие ноты таким образом беспокоиться не должны
					if (pos->LE(cursor))
					{
						Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes = ref new Platform::Collections::Vector < SketchMusic::INote^ > ;
						do
						{
							auto note = dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym);
							if (note)
							{
								notes->Append(note);
							}
							pIter->MoveNext();
						} while (pIter->HasCurrent && pIter->Current->_pos->LE(cursor));
						
						// TODO : конкретизация относительных нот.
						// Они должны конкретизирваться относительно нот вводимых в данный момент
						// Заодно нужно отделить функцию проигрывания текущей позиции / одной доли?,
						// чтобы успешно реализовать пошаговый ввод.
						// Хотя в общем-то ничего больше вводить не надо, пошаговый ввод можно сделать уже на основе имеющихся функций.
						iter->first->Play(notes);

						while (pIter->HasCurrent && !(dynamic_cast<SketchMusic::SNote^>(pIter->Current->_sym)))
						{
							pIter->MoveNext();
							if (!(pIter->HasCurrent)) break;
						}
					} 

					// метроном
					if (needMetronome &&((cursor->getBeat() - pbeat) > 0)) // должно срабатывать, когда переходим на новый бит
					{
						playMetronome();
						pbeat = cursor->getBeat();
					}

					// если уже пропустили символ, то переходим ко следующему, чтобы не возникало "зависаний"
					if ((pIter->HasCurrent) && (pos->LT(cursor)))
					{
						do
						{
							pIter->MoveNext();
							if (!(pIter->HasCurrent)) break;
						} while (!(dynamic_cast<SketchMusic::SNote^>(pIter->Current->_sym)));
					}
				}
			};

			_time64(&ctime); // time64 - размерность 100нс
			//clock_t cl = clock();
			//float offset = (clock()-pclock)/((double)CLOCKS_PER_SEC) * 60 / (double)TICK_IN_BEAT / (double)_BPM;
			auto nClock = Clock::now();
			auto dif = (std::chrono::duration_cast<std::chrono::microseconds>(nClock - pClock).count());
			pClock = nClock;
			float offset = dif * _BPM * TICK_IN_BEAT /60 /1000000;
			//std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
			cursor->incTick(offset);
			//cursor->inc();
			// ждать рассчитанное время
			//unsigned int timeout = 1000 * 60 / TICK_IN_BEAT / _BPM;
			//concurrency::wait(timeout);
				// TODO : отказаться от времени ожидания, вместо этого сразу приступать к дальнейшему воспроизведению,
				// смещая tick курсора на соответствующее разнице во времени положение. Это можно обеспечить, используя float для tick
				// Сейчас, кстати говоря, при расчёте таймаута не принимается во внимание, что произведение всех операций в цикле
				// (не считая всех остальных процессов) занимает какое-то время + нельзя использовать дробные положения (для триолей, например)
			

			if (this->cycling && (tempState == S_STOP))
			{
				tempState = S_PLAY;
				cursor->moveTo(ref new Cursor(0, 0));
				for (auto iter = iterMap->begin(); iter != iterMap->end(); iter++)
				{
					iter->second.first = iter->second.second->First();
				}
			}
		}

	});
	
	getIterators.wait();
	unsigned int timeout = 1000 * 60 / _BPM;
	// для самой последней ноты
	concurrency::wait(timeout);
	this->stop();
	
	// не забываем вернуть все взятые напрокат саунд енжины
	for (auto iter = iterMap->begin(); iter != iterMap->end(); iter++)
	{
		_engines->ReleaseSoundEngine((*iter).first);
	}
	delete iterMap;
}

void SketchMusic::Player::Player::stop()
{
	//cycling = false;
	this->_state = SketchMusic::Player::S_STOP;
}

void SketchMusic::Player::Player::stopKeyboard()
{
	if (_keyboardEngine)
	{
		this->_keyboardEngine->Stop();
	}
}