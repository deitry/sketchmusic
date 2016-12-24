#include "pch.h"
#include "SoundEnginePool.h"
#include "SFSoundEngine.h"
#include "SimpleSoundEngine.h"
#include "Instrument.h"

using namespace SketchMusic::Player;

/**
Вспомогательный метод для публичного GetSoundEngine
*/
ISoundEngine^ SoundEnginePool::GetIfAvailable(Instrument^ instrument)
{
	for (auto iter = _available.begin(); iter != _available.end(); iter++)
	{
		if ((*iter)->getInstrument()->_name == instrument->_name)
		{
			// если доступен, возвращаем, удаляя из списка доступных
			auto result = *iter;
			_available.erase(iter);
			return result;
		}
	}

	return nullptr;
}

ISoundEngine^ SoundEnginePool::GetSoundEngine(Instrument^ instrument)
{
	// добавляем, если такого ещё нет
	AddSoundEngine(instrument);
	
	// проходимся по списку имеющихся
	for (auto engine : _engines)
	{
		if (engine->getInstrument()->_name == instrument->_name)
		{
			// если таковой есть, то смотрим, доступен ли он
			ISoundEngine^ res = GetIfAvailable(instrument);

			// если недоступен
			if (res == nullptr)
			{
				// создаём новую копию этого енжина
				return engine->Copy();
			}
			else return res;
		}
	}

	// если так и не оказалось
	return nullptr;
}

void SoundEnginePool::ReleaseSoundEngine(ISoundEngine^ engine)
{
	engine->Stop();
	// возвращаем в список доступных
	_available.push_back(engine);
}

SoundEnginePool::~SoundEnginePool()
{
	//if (_pool)
	//	delete _pool;
}

/**
Создаёт запрашиваемый саунд енжин и добавляет его в пул доступных
*/
void SoundEnginePool::AddSoundEngine(Instrument^ instrument)
{
	// проверяем, нету ли такого инструмента
	for (auto engine : _engines)
	{
		if (engine->getInstrument()->_name == instrument->_name)
			return;
	}

	ISoundEngine^ engine;
	try
	{
		// пробуем создать на основе саунд фонта
		if (instrument->_name != ref new Platform::String(L"Metronome"))
		{
			engine = ref new SFSoundEngine(_xaudio2, instrument);
		}
	}
	catch (Exception^ e)
	{

	}

	// если создать не удалось, то создаём простой вариант
	// TODO : переместить в catch
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