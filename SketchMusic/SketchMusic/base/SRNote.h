#pragma once 

#include "../SketchMusic.h"

/**
* ������, ������������ ������������� ����.
*
* ������������� ���� ����� ���������� ��������� ������������ ������� �����, ������: 
* �����: 1 - ������, 4 - ������, 4^ - ����������� ������
* �����: 1 - ������, 6 - ������, 7 - ����������� ������
*/
public ref class SketchMusic::SRNote sealed : public ISymbol, public INote
{
private:
	static Platform::String^ valToString(int val);
	// 0 = ������, ����� �� ���������
	// ��������� ������ ����� ���������� � ������� �������� SScale
public:
	SRNote() { _val = 0; }
	SRNote(int rtone) { _val = rtone; _velocity = 0; _voice = 0; }
	SRNote(int rtone, int velocity, int voice) { _val = rtone; _velocity = velocity; _voice = voice; }

	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	virtual SymbolType GetSymType() { return SymbolType::RNOTE; }
	virtual Platform::String^ ToString() { return valToString(_val); }

	// ������������ ����� ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "rnote," + valToString(_val); }
};