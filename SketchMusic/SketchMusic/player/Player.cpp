#include "pch.h"
#include "Player.h"
#include "../SketchMusic.h"
#include "../base/Cursor.h"
#include "../base/Symbol.h"
#include "../base/STempo.h"
#include "../base/SNote.h"
#include "../base/SRNote.h"
#include "../base/SGNote.h"
#include "../base/SScale.h"
#include "../base/Text.h"
#include "../base/Idea.h"
#include "../base/Composition.h"
#include "../base/CompositionData.h"
#include "ISoundEngine.h"
#include "SoundEnginePool.h"
#include "SoundFont.h"
#include "SFSoundEngine.h"
#include "Instrument.h"
#include "agents.h"
#include "ppltasks.h"

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

using namespace Windows::Foundation::Collections;
typedef SketchMusic::Player::PlayerState s;

SketchMusic::Player::Player::Player()
{
	this->_cursor = ref new SketchMusic::Cursor;
	needMetronome = true;
	_BPM = 120;
	_scale = ref new SScale(NoteType::C, ScaleCategory::Minor);
	_harmony = ref new SHarmony(0);
	cycling = false;
	needPlayGeneric = true;
	StopAtLast = true;
	quantize = 4;

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
		if (_keyboardEngine && (!_keyboardEngine->getInstrument()->EQ(instrument)))
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
			_keyboardEngine->Play(note, duration, noteOff);
		}
	});
}


void SketchMusic::Player::Player::playMetronome()
{
	static SNote^ note = ref new SNote(17);
	if (_metronome)
	{
		_metronome->Play(note, 20, nullptr); // TODO : незачем каждый раз создавать новую ноту, можно хранить значение
	}
}


// Для проигрывания идей и прочей мелочи
void SketchMusic::Player::Player::playText(CompositionData^ data, Cursor^ pos) //, SketchMusic::Cursor^ end
{
	playText(data, nullptr, pos);
}

/**
Принимает данные о композиции и проигрывает их начиная с pos (а может и с нуля, в зависимости от настроек)
*/
void SketchMusic::Player::Player::playText(CompositionData^ data, 
										   CompositionLibrary^ lib, 
										   Cursor^ pos)
{
	// - для каждой из дорожек инициализировать инструмент
	// - - создать по соурс войсу на каждый семпл
	// - - если такой инструмент уже проинициализирован, не трогать
	// - пройтись по всем нотам
	this->_state = s::PLAY;

	SketchMusic::Cursor^ cursor = ref new SketchMusic::Cursor(pos);
	_cursor = cursor;

	// ----
	// Этап 1: собираем итераторы со всего набора текстов, которые указывают
	// на ноты, ближайшие к данному положению курсора.
	// Заодно инициализируем SoundEngine, отвечающие за инструменты, связанные с текстом, если таковые ещё не проинициализированы
	auto iterMap 
		= new std::vector< std::pair<SketchMusic::Player::ISoundEngine^, 
									 std::pair<IIterator < SketchMusic::PositionedSymbol^ > ^, 
											   IVector< SketchMusic::PositionedSymbol^ > ^> > >;

		// поиск стартового положения
	auto getIterators = concurrency::create_task([data, iterMap, this, cursor]
	{
		// Обработка основных текстов
		for (auto text : data->texts)
		{
			// получаем список символов в данном тексте
			auto symbols = text->getText();
			// находим ближайший к старту символ (ПОСЛЕ старта)
			
			// с помощью iter мы учитываем все ноты, которые находятся в той точке, откуда мы начинаем
			// startIter обозначает ту самую точку.
			auto iter = symbols->First();
			auto startIter = symbols->First();

			bool initialized = false;
			while (iter->HasCurrent)
			{
				auto p = iter->Current->_pos;
				
				if (p->LT(cursor)) startIter->MoveNext();
				else if (!initialized)
				{
					initialized = true;
					// отыскиваем среди енжинов подходящий
					ISoundEngine^ engine = this->_engines->GetSoundEngine(text->instrument);
					auto cur = startIter->HasCurrent ? startIter->Current : nullptr;
					iterMap->push_back(std::make_pair(engine, std::make_pair(startIter, symbols)));
					break;
				}

				if (p->LE(cursor))
				{
					// обработка системных параметров - учитываем все возможные модуляции
					// темп перенесён в ControlText, так что тут пока ничего нет	

					// проматываем
					iter->MoveNext();

					// если мы достигли последней ноты, а наш курсор ещё дальше
					// TODO : сбрасывать на ноль только если ни в каком из текстов нот дальше чем курсор
					// оставили эту возможность только если включён cycling
					if (cycling && (iter->HasCurrent == false) && (p->LT(cursor)))
					{
						// играем сначала
						cursor->setPos(0);
						iter = symbols->First();
						startIter = symbols->First();
					}
				}
			}
		}

		// Обработка управляющего текста - положение курсора уже определено
		// получаем список символов в данном тексте
		auto symbols = data->ControlText->getText();
		// находим ближайший к старту символ (ПОСЛЕ старта)
		auto iter = symbols->First();
		auto startIter = symbols->First();
		_BPM = data->BPM;
		_scale = data->scale;

		while (iter->HasCurrent)
		{
			auto p = iter->Current->_pos;
		
			if (p->LE(cursor))
			{
				// обработка системных параметров - учитываем все возможные модуляции
				auto tempo = dynamic_cast<STempo^>(iter->Current->_sym);
				if (tempo)
				{
					_BPM = tempo->value;
				}

				auto scale = dynamic_cast<SScale^>(iter->Current->_sym);
				if (scale)
				{
					_scale = scale;
				}

				auto harmony = dynamic_cast<SHarmony^>(iter->Current->_sym);
				if (harmony)
				{
					_harmony = harmony;
				}
		
				// проматываем
				iter->MoveNext();
				if (p->LT(cursor)) startIter->MoveNext();
			}
			else
			{
				// вставляем управляющий текст самым первым, 
				// чтобы он успевал обрабатываться раньше других
				iterMap->insert(iterMap->begin(), 
								std::make_pair(nullptr, std::make_pair(startIter, symbols)));
				break;
			}
		}
	})
		// прекаунт
		.then([this, iterMap, cursor, data]
	{
		if (precount && recording)
		{
			int pbeat = -1;
			clock_t pclock = clock();
			auto pClock = Clock::now();
			int actualCount = precount;
			Cursor^ tmp = ref new Cursor;

			while (actualCount >= 0 && _state != s::STOP)
			{
				// метроном
				if (((tmp->Beat - pbeat) > 0)) // должно срабатывать, когда переходим на новый бит
				{
					playMetronome();
					actualCount--;
				}
				pbeat = tmp->Beat;

				auto nClock = Clock::now();
				auto dif = (std::chrono::duration_cast<std::chrono::nanoseconds>(nClock - pClock).count());
				pClock = nClock;
				float offset = (float)dif * _BPM * TICK_IN_BEAT / 60 / 1000000000;
				tmp->incTick(offset);
			}
		}
	})
		
		// основное проигрывание
		.then([this, iterMap, cursor] //, end
	{
		auto tempState = _state;
		int pbeat = -1;
		
		clock_t pclock = clock();
		auto pClock = Clock::now();

		while (tempState == s::PLAY)
		{
			// запустить асинхронно
			//concurrency::parallel_for_each(iterList->begin(), iterList->end(), [this](Windows::Foundation::Collections::IIterator<SketchMusic::PositionedSymbol^>^ iter)
			// - один итератор = один текст = один инструмент = один саунденжин = несколько нот
			if (StopAtLast) tempState = s::WAIT;
			else tempState = this->_state;

			for (auto iter = iterMap->begin(); iter < iterMap->end(); iter++)
			{
				auto pIter = iter->second.first;
				if (pIter->HasCurrent)
				{
					tempState = this->_state;
					SketchMusic::Cursor^ pos = pIter->Current->_pos;

					// выбираем все ноты в данной точке
					if (pos->LE(cursor))
					{
						auto notes = ref new Platform::Collections::Vector < SketchMusic::INote^ >;
						auto rnotes = ref new Platform::Collections::Vector < SketchMusic::SRNote^ >;
						
						do
						{
							switch (pIter->Current->_sym->GetSymType())
							{
							case SymbolType::NOTE:
							case SymbolType::END:
							{
								auto note = dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym);
								if (note)
								{
									notes->Append(note);
								}
								break;
							}
							case SymbolType::RNOTE:
							{
								auto rnote = dynamic_cast<SketchMusic::SRNote^>(pIter->Current->_sym);
								if (rnote)
								{
									rnotes->Append(rnote);
								}
								break;
							}
							case SymbolType::GNOTE:
							{
								// проигрываем чуть-чуть с помощью метронома, чтобы не заводить отдельный
								auto note = dynamic_cast<SketchMusic::SGNote^>(pIter->Current->_sym);
								if (needPlayGeneric)
									_metronome->Play(ref new SNote(-24 + note->_valX + note->_valY*5), 40, nullptr);
								break;
							}
							case SymbolType::TEMPO:
							{
								auto tempo = dynamic_cast<SketchMusic::STempo^>(pIter->Current->_sym);
								if (tempo) _BPM = tempo->value;
								break;
							}
							case SymbolType::SCALE:
							{
								auto scale = dynamic_cast<SketchMusic::SScale^>(pIter->Current->_sym);
								if (scale) _scale = scale;
								break;
							}
							case SymbolType::HARMONY:
							{
								auto harmony = dynamic_cast<SketchMusic::SHarmony^>(pIter->Current->_sym);
								if (harmony) _harmony = harmony;
								break;
							}
							}
							pIter->MoveNext();
						} while (pIter->HasCurrent && pIter->Current->_pos->LE(cursor));

						for (auto&& rnote : rnotes)
						{
							// конкретизация на основе SScale и SSharmony
							int tone = static_cast<int>(_scale->_baseNote)
								+ _harmony->_val
								+ rnote->_val;

							auto note = ref new SNote(tone);
							if (note)
							{
								// проигрывание
								notes->Append(note);
							}
						}
						
						if (notes && notes->Size > 0 && iter->first)
							iter->first->Play(notes);

						// проматываем до следующего элемента
						// без проверки на INote будем ловить все символы, в т.ч. новые строки и так далее
						// Может, так оно и правильнее.
						//while (pIter->HasCurrent && !(dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym)))
						//{
						//	pIter->MoveNext();
						//	if (!(pIter->HasCurrent)) break;
						//}
					} // if (pos->LE(cursor))

					// если уже пропустили символ, то переходим ко следующему, чтобы не возникало "зависаний"
					if ((pIter->HasCurrent) && (pIter->Current->_pos->LT(cursor)))
					{
						do
						{
							pIter->MoveNext();
							if (!(pIter->HasCurrent)) break;
						} while (!(dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym)));
					}
				} // if (pIter->HasCurrent)
			};
			
			// метроном
			if (needMetronome && ((cursor->Beat - pbeat) > 0)) // должно срабатывать, когда переходим на новый бит
			{
				playMetronome();
			}
			pbeat = cursor->Beat;

			auto nClock = Clock::now();
			auto dif = (std::chrono::duration_cast<std::chrono::microseconds>(nClock - pClock).count());
			pClock = nClock;
			float offset = dif * _BPM * TICK_IN_BEAT / 60 / 1000000;
			cursor->incTick(offset);

			// квантизация
			static int prevQuant = -1;
			static int prevBeat = -1;

			int localQ = quantize ? quantize : 32;
			int quant = static_cast<int>(cursor->Tick * localQ / TICK_IN_BEAT);
			
			int beat = cursor->Beat;
			if ((quant != prevQuant) || (beat != prevBeat))
			{
				Cursor^ pos = ref new Cursor(cursor);
				pos->Tick = (float) quant * TICK_IN_BEAT / localQ;	// округляем до ближайшего кванта
				CursorPosChanged(this, pos);
				prevQuant = quant;
				prevBeat = beat;
			}

			// остановка у конца - простой вариант
			//if (end)
			//{
			//	if (end->LE(cursor) && !cycling)
			//	{
			//		tempState == s::STOP;
			//	}
			//}

			if (this->cycling && (tempState == s::WAIT))
			{
				tempState = s::PLAY;
				pbeat = -1;
				cursor->moveTo(ref new Cursor(0, 0));
				for (auto iter = iterMap->begin(); iter != iterMap->end(); iter++)
				{
					iter->second.first = iter->second.second->First();
				}
			}
		}

	});
	
	getIterators.wait();
	unsigned int timeout = static_cast<unsigned int>(1000 * 60 / _BPM);
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
	this->_state = s::STOP;
}

void SketchMusic::Player::Player::stopKeyboard()
{
	if (_keyboardEngine)
	{
		this->_keyboardEngine->Stop();
	}
}

// Запрашиваем у СаундПула данные о SF, чтобы иметь возможность их обрабатывать - например, получать список пресетов
SketchMusic::SFReader::SFData ^ SketchMusic::Player::Player::GetSFData(SketchMusic::Instrument ^ instrument)
{
	auto sf2engine = dynamic_cast<SketchMusic::Player::SFSoundEngine^>(_engines->GetSoundEngine(instrument));
	if (sf2engine)
	{
		return sf2engine->_soundFontData;
	}
	return nullptr;
}
