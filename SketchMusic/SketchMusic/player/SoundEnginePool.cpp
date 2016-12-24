#include "pch.h"
#include "SoundEnginePool.h"
#include "SFSoundEngine.h"
#include "SimpleSoundEngine.h"
#include "Instrument.h"

using namespace SketchMusic::Player;

/**
��������������� ����� ��� ���������� GetSoundEngine
*/
ISoundEngine^ SoundEnginePool::GetIfAvailable(Instrument^ instrument)
{
	for (auto iter = _available.begin(); iter != _available.end(); iter++)
	{
		if ((*iter)->getInstrument()->_name == instrument->_name)
		{
			// ���� ��������, ����������, ������ �� ������ ���������
			auto result = *iter;
			_available.erase(iter);
			return result;
		}
	}

	return nullptr;
}

ISoundEngine^ SoundEnginePool::GetSoundEngine(Instrument^ instrument)
{
	// ���������, ���� ������ ��� ���
	AddSoundEngine(instrument);
	
	// ���������� �� ������ ���������
	for (auto engine : _engines)
	{
		if (engine->getInstrument()->_name == instrument->_name)
		{
			// ���� ������� ����, �� �������, �������� �� ��
			ISoundEngine^ res = GetIfAvailable(instrument);

			// ���� ����������
			if (res == nullptr)
			{
				// ������ ����� ����� ����� ������
				return engine->Copy();
			}
			else return res;
		}
	}

	// ���� ��� � �� ���������
	return nullptr;
}

void SoundEnginePool::ReleaseSoundEngine(ISoundEngine^ engine)
{
	engine->Stop();
	// ���������� � ������ ���������
	_available.push_back(engine);
}

SoundEnginePool::~SoundEnginePool()
{
	//if (_pool)
	//	delete _pool;
}

/**
������ ������������� ����� ����� � ��������� ��� � ��� ���������
*/
void SoundEnginePool::AddSoundEngine(Instrument^ instrument)
{
	// ���������, ���� �� ������ �����������
	for (auto engine : _engines)
	{
		if (engine->getInstrument()->_name == instrument->_name)
			return;
	}

	ISoundEngine^ engine;
	try
	{
		// ������� ������� �� ������ ����� �����
		if (instrument->_name != ref new Platform::String(L"Metronome"))
		{
			engine = ref new SFSoundEngine(_xaudio2, instrument);
		}
	}
	catch (Exception^ e)
	{

	}

	// ���� ������� �� �������, �� ������ ������� �������
	// TODO : ����������� � catch
	if (engine == nullptr)
	{
		engine = ref new SimpleSoundEngine(_xaudio2, instrument);
	}

	_engines.push_back(engine);
	_available.push_back(engine);
}

void SoundEnginePool::DeleteSoundEngine(Instrument^ instrument)
{
	for (auto iter = _engines.begin(); iter != _engines.end(); iter++)
	{
		if ((*iter)->getInstrument()->_name == instrument->_name)
		{
			_engines.erase(iter);
		}
	}
}