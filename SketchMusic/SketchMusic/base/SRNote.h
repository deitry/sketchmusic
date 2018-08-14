#pragma once 

#include "../SketchMusic.h"

/**
* Символ, обозначающий относительную ноту.
*
* Относительная нота задаёт конкретное положение относительно текущей гаммы, пример: 
* текст: 1 - тоника, 4 - кварта, 4^ - увеличенная кварта
* число: 1 - тоника, 6 - кварта, 7 - увеличенная кварта
*/
public ref class SketchMusic::SRNote sealed : public ISymbol, public INote
{
private:
	static Platform::String^ valToString(int val);
	// 0 = тоника, далее по полутонам
	// положение тоники будет задаваться с помощью символов SScale
public:
	SRNote() { _val = 0; }
	SRNote(int rtone) { _val = rtone; _velocity = 0; _voice = 0; }
	SRNote(int rtone, int velocity, int voice) { _val = rtone; _velocity = velocity; _voice = voice; }

	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	virtual SymbolType GetSymType() { return SymbolType::RNOTE; }
	virtual Platform::String^ ToString() { return valToString(_val); }

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "rnote," + valToString(_val); }
};