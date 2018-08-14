#pragma once

#include "../SketchMusic.h"

public ref class SketchMusic::STempo sealed : public SketchMusic::ISymbol
{
public:
	STempo() {}
	STempo(float t) { value = t; }

	property float value;

	virtual SymbolType GetSymType() { return SymbolType::TEMPO; }
	virtual Platform::String^ ToString() { return value.ToString(); }

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second);
};