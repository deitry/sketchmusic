#pragma once

#include "../SketchMusic.h"

/**
* Символ, обозначающий прекращение звучания текущей ноты.
*
* Так как система предполагает задавать не продолжительность нот, а точки начала, необходимо отмечать завершение,
* если оно требуется.
*/
public ref class SketchMusic::SNoteEnd sealed : public ISymbol, public INote
{
public:
	SNoteEnd() {}

	virtual SymbolType GetSymType() { return SymbolType::END; }
	virtual Platform::String^ ToString() { return L"\uE81A"; }

	// Унаследовано через INote
	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "end"; }
};