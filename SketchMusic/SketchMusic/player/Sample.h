#pragma once

#include "../SketchMusic.h"
#include "xaudio2.h"

/**
Определяет одиночный семпл.
Он будет храниться в файле.
*/
public ref class SketchMusic::Sample sealed
{
private:
	Platform::String^ _name;	// наименование
	int _length;				// длина семпла - может быть вычислена? исходя из высоты и заданной частоты дискретизации
	float _frequency;			// высота тона - частота, соответствующая данному семплу
	Platform::Array<uint8>^ _sample;	// собственно сам семпл
	~Sample();

public:
	Sample(Platform::String^ FileName);	// подгрузить семпл из файла
		// - или по названию искать?

	Platform::Array<uint8>^ getSample() { return _sample; }
	int getLength() { return _length; }
	float getFrequency() { return _frequency; }
	Platform::String^ getName() { return _name; }
};