#pragma once

#include "../SketchMusic.h"

using namespace SketchMusic;
using namespace SketchMusic::View;

public ref class SketchMusic::View::Key sealed
{
protected:
	void UpdateState(KeyboardState^ keyboard)
	{
		switch (keyboard->state)
		{
		case KeyboardStateEnum::octaveInc:
			switch (this->type)
			{
			case KeyType::note:
			case KeyType::relativeNote:
				this->value += 12;
				break;
			}

			break;
		case KeyboardStateEnum::octaveDec:
			switch (this->type)
			{
			case KeyType::note:
			case KeyType::relativeNote:
				this->value -= 12;
				break;
			}
			break;
		}
	}
public:
	property KeyType type;
	property int value;
	property int shift;	// ранее известно как striked - аналог повышения регистра при нажатии шифта
	property Object^ parent;

	Key(int keyNum)
	{
		// переводим из числа в клавишу согласно "раскладке"
		// последние три числа идут как значение
		// передние определяют тип
		// int = TTNNN
		// тип сходится с числовым представлением KeyType
		type = (KeyType)abs(keyNum / 1000);
		value = keyNum % 1000;
		shift = 0;
	}

	Key(KeyType type, int value)
	{
		this->type = type;
		this->value = value;
		this->shift = 0;
	}
};

template<>
struct std::less<SketchMusic::View::Key^>
{
	bool operator() (SketchMusic::View::Key^ x, SketchMusic::View::Key^ y) const { return x->type < y->type; }
	typedef SketchMusic::View::Key^ first_argument_type;
	typedef SketchMusic::View::Key^ second_argument_type;
	typedef bool result_type;
};