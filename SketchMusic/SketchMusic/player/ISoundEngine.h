#pragma once

#include "../SketchMusic.h"

/**
Отвечает за звучание определённого инструмента.
*/
interface class SketchMusic::Player::ISoundEngine
{
public:

	ISoundEngine^ Copy();

	void Play(SketchMusic::INote^ note, int duration, NoteOff^ noteOff); // duration в тиках
		// возможно, стоит разделить на две отдельных функции

	// нужна для проигрывания отдельных нот, например при выделении или вводе с клавиатуры
	void Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes);
		// нужна для проигрывания сразу нескольких нот, например, при воспроизведении текста
	void Stop();
	SketchMusic::Instrument^ getInstrument();
	void SetPreset(Platform::String^ preset);
	//void setEffect();

	// функции по работе с формой волны? Изменение атаки, затухания и т.д. - изменение буфера
};