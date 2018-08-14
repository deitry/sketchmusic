#pragma once

#include "../SketchMusic.h"

/**
* ������, ������������ ���������� ����.
*
* ���������� ���� ������ ������ ����� ���� ������������ ������.
* �� ��������� �������� ����� ������������ �������� "���������� ��������",
* - �����, ��� �������������, ��� ����� ��������� ����� ������,
* - ��� ����� ����� ��������� ����������� ����� ��������.
*/
public ref class SketchMusic::SGNote sealed : public SketchMusic::ISymbol, public SketchMusic::INote
{
private:
	static Platform::String^ valToString(int val);
	// ����� �� �������������.
	// � ���������� ��� ����������� �������������� ������������� ����� ��������� �� "��������" (?)	
public:
	SGNote() {}
	SGNote(int gnote) { _val = gnote; _velocity = 0; _voice = 0; }
	SGNote(int gtone, int velocity, int voice) { _val = gtone; _velocity = velocity; _voice = voice; }

	virtual property int _val
	{
		int get() { return (abs(_valX) + _valY * 100)* ((_valX >= 0) ? 1 : -1); }
		void set(int val) { _valY = abs(val) / 100; _valX = val % 100; }
	}

	property int _valX;
	property int _valY;
	virtual property int _velocity;
	virtual property int _voice;

	virtual SymbolType GetSymType() { return SymbolType::GNOTE; }
	virtual Platform::String^ ToString() { return L"" + _valX + ";" + _valY; }

	// ������������ ����� ISymbol
	virtual bool EQ(ISymbol ^ second);
	//virtual Platform::String^ Serialize() { return "gnote," + valToString(_val); }
};