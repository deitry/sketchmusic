#include "pch.h"
#include "SoundEnginePool.h"
#include "SFSoundEngine.h"
#include "SimpleSoundEngine.h"
#include "Instrument.h"
#include "SoundFont.h"

using namespace SketchMusic::Player;

/**
��������������� ����� ��� ���������� GetSoundEngine
*/
ISoundEngine^ SoundEnginePool::GetIfAvailable(Instrument^ instrument)
{
	for (auto&& iter : _available)
	{
		if (iter.first->getInstrument()->_name == instrument->_name && iter.second)
		{
			// ���� ��������, ����������, ������ �� ������ ���������
			auto result = iter.first;
			iter.second = false;
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
				res = engine->Copy();
			}
			// ������������� ������, ��������������� ���������� �����������
			res->SetPreset(instrument->preset);
			return res;
		}
	}

	// ���� ��� � �� ���������
	return nullptr;
}

void SoundEnginePool::ReleaseSoundEngine(ISoundEngine^ engine)
{
	if (engine == nullptr) return;

	engine->Stop();
	// ���������� � ������ ���������
	for (auto&& iter : _available)
	{
		if (iter.first == engine)
		{
			iter.second = true;
			break;
		}
	}
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
		// ����� ��������� ������ �� �����
		if (engine->getInstrument()->_name == instrument->_name)
			return;
	}

	ISoundEngine^ engine;
	
	// ������� ������� �� ������ ����� �����
	if (instrument->_name != ref new Platform::String(L"Metronome"))
	{
		engine = SFSoundEngine::GetSFSoundEngine(_xaudio2, instrument);
	}
	

	// ���� ������� �� �������, �� ������ ������� �������
	// TODO : ����������� � catch
	if (engine == nullptr)
	{
		engine = ref new SimpleSoundEngine(_xaudio2, instrument);
	}

	_engines.push_back(engine);
	_available.push_back(std::make_pair(engine,true));
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