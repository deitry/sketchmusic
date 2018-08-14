#pragma once

#include "../SketchMusic.h"

/**
* ������, ������������ ����������� �������� ������� ����.
*
* ��� ��� ������� ������������ �������� �� ����������������� ���, � ����� ������, ���������� �������� ����������,
* ���� ��� ���������.
*/
public ref class SketchMusic::SNoteEnd sealed : public ISymbol, public INote
{
public:
	SNoteEnd() {}

	virtual SymbolType GetSymType() { return SymbolType::END; }
	virtual Platform::String^ ToString() { return L"\uE81A"; }

	// ������������ ����� INote
	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	// ������������ ����� ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "end"; }
};