#pragma once

#include "../SketchMusic.h"

public ref class SketchMusic::SNewPart sealed : public SketchMusic::ISymbol
{
public:
	SNewPart() {}
	SNewPart(String^ _cat) { category = _cat; }
	SNewPart(String^ _name, String^ _cat, DynamicCategory _dyn) { Name = _name; category = _cat; dynamic = _dyn; }

	property String^ Name;
	property String^ category;	// "A", "B", "������", "�����" - ������������ ������ ���������
								//, ����� ����� ������������ ������������� � ����������� �� ��������� � ����������

	property DynamicCategory dynamic;

	virtual SymbolType GetSymType() { return SymbolType::NPART; }
	virtual Platform::String^ ToString() { return L"\uE70B" + category; }

	// ������������ ����� ISymbol
	virtual bool EQ(ISymbol ^ second);
};