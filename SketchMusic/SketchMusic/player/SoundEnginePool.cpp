#include "pch.h"
#include "SoundEnginePool.h"
#include "SFSoundEngine.h"
#include "SimpleSoundEngine.h"
#include "Instrument.h"
#include "SoundFont.h"

using namespace SketchMusic::Player;

/**
Вспомогательный метод для публичного GetSoundEngine
*/
ISoundEngine^ SoundEnginePool::GetIfAvailable(Instrument^ instrument)
{
	for (auto&& iter : _available)
	{
		if (iter.first->getInstrument()->_name == instrument->_name && iter.second)
		{
			// если доступен, возвращаем, удаляя из списка доступных
			auto result = iter.first;
			iter.second = false;
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
				res = engine->Copy();
			}
			// устанавливаем пресет, соответствующий выбранному инструменту
			res->SetPreset(instrument->preset);
			return res;
		}
	}

	// если так и не оказалось
	return nullptr;
}

void SoundEnginePool::ReleaseSoundEngine(ISoundEngine^ engine)
{
	if (engine == nullptr) return;

	engine->Stop();
	// возвращаем в список доступных
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
Создаёт запрашиваемый саунд енжин и добавляет его в пул доступных
*/
void SoundEnginePool::AddSoundEngine(Instrument^ instrument)
{
	// проверяем, нету ли такого инструмента
	for (auto engine : _engines)
	{	
		// здесь пробиваем только по имени
		if (engine->getInstrument()->_name == instrument->_name)
			return;
	}

	ISoundEngine^ engine;
	
	// пробуем создать на основе саунд фонта
	if (instrument->_name != ref new Platform::String(L"Metronome"))
	{
		engine = SFSoundEngine::GetSFSoundEngine(_xaudio2, instrument);
	}
	

	// если создать не удалось, то создаём простой вариант
	// TODO : переместить в catch
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