#pragma once

#include "xaudio2.h"
#include "../SketchMusic.h"
#include "ISoundEngine.h"

using namespace SketchMusic;
using namespace SketchMusic::Player;

/**
Класс позволяет отделить реализацию и инициализацию СаундЕнжинов от их использования в плеере.

TODO : Может вообще запретить плееру пользоваться непосредственно саунденжинами и предоставить 
только функции проигрывания и остановки с инструментом в качестве параметра?
*/
ref class SketchMusic::Player::SoundEnginePool sealed
{
private:
	std::list<ISoundEngine^> _engines;
	std::list<std::pair<ISoundEngine^, bool>> _available;		// для отображения доступных саунденжинов
		// ! - количество доступных будет больше, чем количество всех, потому что туда будут добавляться копии
	
	Microsoft::WRL::ComPtr<IXAudio2> _xaudio2;	// нужен для успешного создания саунденжинов
	ISoundEngine^ _default;	// енжин, который будет возращаться по умолчанию

	ISoundEngine^ GetIfAvailable(Instrument^ instrument);
	~SoundEnginePool();
internal:
	SoundEnginePool(Microsoft::WRL::ComPtr<IXAudio2> xaudio2) { _xaudio2 = xaudio2; }

	ISoundEngine^ GetSoundEngine(Instrument^ instrument);
	void ReleaseSoundEngine(ISoundEngine^ engine);

	void AddSoundEngine(Instrument^ instrument);
	void DeleteSoundEngine(Instrument^ instrument);
};