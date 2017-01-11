#include "pch.h"
#include "Player.h"
#include "../SketchMusic.h"
#include "../base/Cursor.h"
#include "../base/Symbol.h"
#include "../base/Text.h"
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
	cycling = false;
	needPlayGeneric = true;
	quantize = 4.;

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
			_keyboardEngine->Play(note, 0, noteOff);
		}
	});
}


void SketchMusic::Player::Player::playMetronome()
{
	static SNote^ note = ref new SNote(17);
	if (_metronome)
	{
		_metronome->Play(note, 20, nullptr); // TODO : ������� ������ ��� ��������� ����� ����, ����� ������� ��������
	}
}

/**
��������� ����� � ����������� ��� ������� �� start ���� �� �����, ���� ���� �� �����
������� ������������.
*/
void SketchMusic::Player::Player::playText(CompositionData^ data, SketchMusic::Cursor^ pos)
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

	auto getIterators = concurrency::create_task([data, iterMap, this, cursor]
	{
		for (auto text : data->texts)
		{
			// �������� ������ �������� � ������ ������
			auto symbols = text->getText();
			// ������� ��������� � ������ ������ (����� ������)
			
			auto iter = symbols->First();
			auto startIter = symbols->First();

			_BPM = 120;
			while (iter->HasCurrent)
			{
				auto p = iter->Current->_pos;
				
				if (p->LE(cursor))
				{
					// ��������� ��������� ���������� - ��������� ��� ��������� ���������
					auto tempo = dynamic_cast<STempo^>(iter->Current->_sym);
					if (tempo)
					{
						_BPM = tempo->value;
					}

					// �����������
					iter->MoveNext();
					if (p->LT(cursor)) startIter->MoveNext();

					// ���� �� �������� ��������� ����, � ��� ������ ��� ������
					if ((iter->HasCurrent == false) && (p->LE(cursor)))
					{
						// ������ �������
						cursor->setPos(0);
						iter = symbols->First();
						startIter = symbols->First();
					}
				}
				else
				{
					// ���������� ����� ������� ����������
					ISoundEngine^ engine = this->_engines->GetSoundEngine(text->instrument);
					
					iterMap->push_back(std::make_pair(engine, std::make_pair(startIter, symbols)));
					break;
				}
			}
		}
	})
		.then([this, iterMap, cursor, data]
	{
		// ��������
		if (precount && recording)
		{
			int pbeat = -1;
			clock_t pclock = clock();
			auto pClock = Clock::now();
			int actualCount = precount;
			Cursor^ tmp = ref new Cursor;

			while (actualCount > 0 && _state != s::STOP)
			{
				// ��������
				if (((tmp->getBeat() - pbeat) > 0)) // ������ �����������, ����� ��������� �� ����� ���
				{
					playMetronome();
					actualCount--;
				}
				pbeat = tmp->getBeat();

				auto nClock = Clock::now();
				auto dif = (std::chrono::duration_cast<std::chrono::microseconds>(nClock - pClock).count());
				pClock = nClock;
				float offset = (float)dif * _BPM*1.5 * TICK_IN_BEAT / 60 / 1000000;	// �� ����, ������, �� ��� ����������� ��������������� ���� �������� �� �������
				tmp->incTick(offset);
			}
		}
	})
		.then([this, iterMap, cursor]
	{
		auto tempState = _state;
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
						Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes = ref new Platform::Collections::Vector < SketchMusic::INote^ >;
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
							case SymbolType::GNOTE:
							{
								// ����������� ����-���� � ������� ���������, ����� �� �������� ���������
								auto note = dynamic_cast<SketchMusic::SGNote^>(pIter->Current->_sym);
								if (needPlayGeneric)
									_metronome->Play(ref new SNote(-24 + note->_valX + note->_valY*5), 40, nullptr);
								break;
							}
							case SymbolType::TEMPO:
							{
								auto tempo = dynamic_cast<SketchMusic::STempo^>(pIter->Current->_sym);
								if (tempo)
								{
									_BPM = tempo->value;
								}
							}
							}
							pIter->MoveNext();
						} while (pIter->HasCurrent && pIter->Current->_pos->LE(cursor));

						// TODO : ������������� ������������� ���.
						// ��� ������ ����������������� ������������ ��� �������� � ������ ������
						// ������ ����� �������� ������� ������������ ������� ������� / ����� ����?,
						// ����� ������� ����������� ��������� ����.
						// ���� � �����-�� ������ ������ ������� �� ����, ��������� ���� ����� ������� ��� �� ������ ��������� �������.
						iter->first->Play(notes);

						// ����������� �� ���������� ��������
						// ��� �������� �� INote ����� ������ ��� �������, � �.�. ����� ������ � ��� �����
						// �����, ��� ��� � ����������.
						//while (pIter->HasCurrent && !(dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym)))
						//{
						//	pIter->MoveNext();
						//	if (!(pIter->HasCurrent)) break;
						//}
					}

					// ��������
					if (needMetronome && ((cursor->getBeat() - pbeat) > 0)) // ������ �����������, ����� ��������� �� ����� ���
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
						} while (!(dynamic_cast<SketchMusic::INote^>(pIter->Current->_sym)));
					}
				}
			};

			auto nClock = Clock::now();
			auto dif = (std::chrono::duration_cast<std::chrono::microseconds>(nClock - pClock).count());
			pClock = nClock;
			float offset = dif * _BPM * TICK_IN_BEAT / 60 / 1000000;
			cursor->incTick(offset);

			// �����������
			if (quantize)
			{
				static int prevQuant = -1;
				static int prevBeat = -1;
				int quant = cursor->getTick() * quantize / TICK_IN_BEAT;
				int beat = cursor->getBeat();
				if ((quant != prevQuant) || (beat != prevBeat))
				{
					CursorPosChanged(this, cursor);
					prevQuant = quant;
					prevBeat = beat;
				}
			} 
			//else { // ��� ����������� �������� ������ ���
			//	CursorPosChanged(this, cursor);
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

// ����������� � ��������� ������ � SF, ����� ����� ����������� �� ������������ - ��������, �������� ������ ��������
SketchMusic::SFReader::SFData ^ SketchMusic::Player::Player::GetSFData(SketchMusic::Instrument ^ instrument)
{
	auto sf2engine = dynamic_cast<SketchMusic::Player::SFSoundEngine^>(_engines->GetSoundEngine(instrument));
	if (sf2engine)
	{
		return sf2engine->_soundFontData;
	}
	return nullptr;
}
