#pragma once 

#include "../SketchMusic.h"

namespace
{
	Platform::String^ auxMod = "*";
}

/**
* Символ, обозначающий относительную ноту.
*
* Относительная нота задаёт конкретное положение относительно текущей гаммы, пример: 
* текст: 1 - тоника, 4 - кварта, 4^ - увеличенная кварта
* число: 1 - тоника, 6 - кварта, 7 - увеличенная кварта
*/
public ref class SketchMusic::SRNote sealed : public ISymbol, public INote
{
public:
	SRNote() { _val = 0; }
	SRNote(int rtone) { _val = rtone; _velocity = 0; _voice = 0; }
	SRNote(int rtone, int velocity, int voice) { _val = rtone; _velocity = velocity; _voice = voice; }

	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	virtual SymbolType GetSymType() { return SymbolType::RNOTE; }
	virtual Platform::String^ ToString() 
	{ 
		String^ str = (_val < 0) ? "-" : "";
		// FIXME: подумать
		// разрешаем относительные на плюс-минус две октавы
		// TODO: отображение должно меняться в зависимости от текущей гаммы
		// \u0303 - тильда
		// \u0308 - двоеточие
		// сейчас расчитано на минор
		switch (abs(_val) % 24)
		{
			// первая октава
			case 0: str += "T"; break;
			case 1: str += "1" + ::auxMod; break;
			case 2: str += "2"; break;
			case 3: str += "3"; break;
			case 4: str += "3" + ::auxMod; break;
			case 5: str += "4"; break;
			case 6: str += "4" + ::auxMod; break;
			case 7: str += "5"; break;
			case 8: str += "6"; break;
			case 9: str += "6" + ::auxMod; break;
			case 10: str += "7"; break;
			case 11: str += "7" + ::auxMod; break;
			// вторая октава
			case 12: str += "t"; break;
			case 13: str += "8" + ::auxMod; break;
			case 14: str += "9"; break;
			case 15: str += "10"; break;
			case 16: str += "10" + ::auxMod; break;
			case 17: str += "11"; break;
			case 18: str += "11" + ::auxMod; break;
			case 19: str += "12"; break;
			case 20: str += "13"; break;
			case 21: str += "13" + ::auxMod; break;
			case 22: str += "14"; break;
			case 23: str += "14" + ::auxMod; break;
			case 24: str += "tt"; break;
			default: break;
		}
		//str += ((_val + MIDIKEYTOA4 - 9) / 12 - 1);
		return str;
	}

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second);
};