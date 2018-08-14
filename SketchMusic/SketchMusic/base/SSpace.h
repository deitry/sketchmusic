#pragma once

#include "../SketchMusic.h"
/**
* Символ, обозначающий свободное место.
*
* Никак не будет влиять на воспроизведение, нужен только для визуального отображения.
*/
public ref class SketchMusic::SSpace sealed : public SketchMusic::ISymbol
{
public:
	SSpace() {}

	virtual SymbolType GetSymType() { return SymbolType::SPACE; }
	virtual Platform::String^ ToString() { return L"\uE75D"; }

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "space"; }
};