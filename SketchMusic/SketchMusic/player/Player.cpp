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
typedef SketchMusic::Player::PlayerState s;

SketchMusic::Player::Player::Player()
{
	this->_cursor = ref new SketchMusic::Cursor;
	needMetronome = true;
	_BPM = 120;
	cycling = false;

	// do - ����� ����� ���� "��������" �������������, ���� ��-�� �� �������
	do
	{
		// ������ ������ XAudio2
		HRESULT hr = XAudio2Create(&pXAudio2);
		if (FAILED(hr))
		{
			//CurPos->Text = "XAudio2Create failure: " + hr.ToString();
			break;
		}

		// ������ ������������� �����
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
����� ����������� ��������� ���� � ������������������ �� ���������.
������������ ��� ������������ ���� ��� ������� �� ����������
��������, ����� ���������� � ������� playNotes ��� ������� �����������...
*/
void SketchMusic::Player::Player::playSingleNote(SketchMusic::INote^ note, SketchMusic::Instrument^ instrument, int duration, SketchMusic::Player::NoteOff^ noteOff)
{
	auto playNote = concurrency::create_task([=]
	{
		// ���� ��������� �� ����������
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
		_metronome->Play(ref new SNote(17), 20, nullptr); // TODO : ������� ������ ��� ��������� ����� ����, ����� ������� ��������
	}
}

/**
��������� ����� � ����������� ��� ������� �� start ���� �� �����, ���� ���� �� �����
������� ������������.
*/
void SketchMusic::Player::Player::playText(Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ texts, SketchMusic::Cursor^ pos)
{
	// - ��� ������ �� ������� ���������������� ����������
	// - - ������� �� ����� ����� �� ������ �����
	// - - ���� ����� ���������� ��� ������������������, �� �������
	// - �������� �� ���� �����
	this->_state = s::PLAY;
	

	SketchMusic::Cursor^ cursor = ref new SketchMusic::Cursor(pos);
	//_cursor->moveTo(pos);

	// ----
	// ���� 1: �������� ��������� �� ����� ������ �������, ������� ���������
	// �� ����, ��������� � ������� ��������� �������.
	// ������ �������������� SoundEngine, ���������� �� �����������, ��������� � �������, ���� ������� ��� �� �������������������
	auto iterMap 
		= new std::vector< std::pair<SketchMusic::Player::ISoundEngine^, std::pair<IIterator < SketchMusic::PositionedSymbol^ > ^, IVector< SketchMusic::PositionedSymbol^ > ^> > >;

	auto getIterators = concurrency::create_task([texts, iterMap, this, cursor]
	{
		for (auto text : texts)
		{
			
			// �������� ������ �������� � ������ ������
			auto symbols = text->getText();
			// ������� ��������� � ������ ������ (����� ������)
			
			// TODO : �������� ����� ����������� ��������� � ������ (����� �������)
			// ������ ������ ������ - ��� ����� ��������� ���������� � �����, ����� � �.�.
			auto iter = symbols->First();
			while (iter->HasCurrent)
			{
				if (iter->Current->_pos->LT(cursor))
				{
					iter->MoveNext();
				}
				else
				{
					// ���������� ����� ������� ����������
					ISoundEngine^ engine = this->_engines->GetSoundEngine(text->instrument);
					
					iterMap->push_back(std::make_pair(engine, std::make_pair(iter, symbols)));
					break;
				}
			}
		}
	})
		.then([this, iterMap, cursor]
	{
		auto tempState = s::PLAY;
		int pbeat = -1;
		
		clock_t pclock = clock();
		auto pClock = Clock::now();

		while (tempState == s::PLAY)
		{
			// ��������� ����������
			//concurrency::parallel_for_each(iterList->begin(), iterList->end(), [this](Windows::Foundation::Collections::IIterator<SketchMusic::PositionedSymbol^>^ iter)
			// - ���� �������� = ���� ����� = ���� ���������� = ���� ���������� = ��������� ���
			tempState = s::WAIT;
			for (auto iter = iterMap->begin(); iter < iterMap->end(); iter++)
			{
				auto pIter = iter->second.first;
				if (pIter->HasCurrent)
				{
					tempState = this->_state;
					SketchMusic::Cursor^ pos = pIter->Current->_pos;
					
					// �������� ��� ���� � ������ �����
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
						
						// TODO : ������������� ������������� ���.
						// ��� ������ ����������������� ������������ ��� �������� � ������ ������
						// ������ ����� �������� ������� ������������ ������� ������� / ����� ����?,
						// ����� ������� ����������� ��������� ����.
						// ���� � �����-�� ������ ������ ������� �� ����, ��������� ���� ����� ������� ��� �� ������ ��������� �������.
						iter->first->Play(notes);

						while (pIter->HasCurrent && !(dynamic_cast<SketchMusic::SNote^>(pIter->Current->_sym)))
						{
							pIter->MoveNext();
							if (!(pIter->HasCurrent)) break;
						}
					} 

					// ��������
					if (needMetronome &&((cursor->getBeat() - pbeat) > 0)) // ������ �����������, ����� ��������� �� ����� ���
					{
						playMetronome();
					}
					pbeat = cursor->getBeat();

					// ���� ��� ���������� ������, �� ��������� �� ����������, ����� �� ��������� "���������"
					if ((pIter->HasCurrent) && (pIter->Current->_pos->LT(cursor)))
					{
						do
						{
							pIter->MoveNext();
							if (!(pIter->HasCurrent)) break;
						} while (!(dynamic_cast<SketchMusic::SNote^>(pIter->Current->_sym)));
					}
				}
			};

			auto nClock = Clock::now();
			auto dif = (std::chrono::duration_cast<std::chrono::microseconds>(nClock - pClock).count());
			pClock = nClock;
			float offset = dif * _BPM * TICK_IN_BEAT /60 /1000000;
			cursor->incTick(offset);
			

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
	unsigned int timeout = 1000 * 60 / _BPM;
	// ��� ����� ��������� ����
	concurrency::wait(timeout);
	this->stop();
	
	// �� �������� ������� ��� ������ �������� ����� ������
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