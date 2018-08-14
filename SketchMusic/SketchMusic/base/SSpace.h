#pragma once

#include "../SketchMusic.h"
/**
* ������, ������������ ��������� �����.
*
* ����� �� ����� ������ �� ���������������, ����� ������ ��� ����������� �����������.
*/
public ref class SketchMusic::SSpace sealed : public SketchMusic::ISymbol
{
public:
	SSpace() {}

	virtual SymbolType GetSymType() { return SymbolType::SPACE; }
	virtual Platform::String^ ToString() { return L"\uE75D"; }

	// ������������ ����� ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "space"; }
};