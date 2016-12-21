#pragma once

#include "../SketchMusic.h"
#include "rapidjson/prettywriter.h"

[Windows::UI::Xaml::Data::Bindable]
public ref class SketchMusic::Cursor sealed
{
private:
	int _beat;	// ����� "��������" ����.
				// ��������� �� ����� ���������� � ������������
				// � ���������� ���������.
				// ���� ���������� ����� ���������� ������ ������������ ������� �����.
	int _tick;	// ����� "�������" ����. ������ �������, ��� ������� �����,
				// ���� ����� �������, ��� � ����� ������� ���� 256 ������

public:	
	// ������������
	Cursor() {_beat = 0; _tick = 0; }
	Cursor(int b) { _beat = b; _tick = 0; }
	Cursor(int b, int t) { _beat = b; _tick = t; }
	Cursor(Cursor^ cur) { _beat = cur->_beat; _tick = cur->_tick; }

	// ������ � �����
	
	void moveTo(SketchMusic::Cursor^ pos) { _beat = pos->getBeat(); _tick = pos->getTick(); }
	void setPos(int b, int t) { _beat = b; _tick = t; }
	void setPos(int b) { setPos(b, 0); }

	int getBeat() { return _beat; }
	int getTick() { return _tick; }
	int toTicks() { return _tick + _beat*SketchMusic::TICK_IN_BEAT; }

	// ��������

	SketchMusic::Cursor^ inc();
	SketchMusic::Cursor^ incBeat()
	{
		this->_beat++;
		this->_tick = 0;
		return ref new SketchMusic::Cursor(_beat,0);
	}
	SketchMusic::Cursor^ dec();
	SketchMusic::Cursor^ decBeat()
	{
		if (this->_beat > 0)
			this->_beat--;
		this->_tick = 0;
		return ref new SketchMusic::Cursor(_beat, 0);
	}
	
	bool EQ(SketchMusic::Cursor^ that);
	bool GT(SketchMusic::Cursor^ that);
	bool LT(SketchMusic::Cursor^ that);

	Platform::String^ ToString() { return "" + _beat + ":" + _tick; }
	
	property Platform::String^ asString;
};

template<>
struct std::less<SketchMusic::Cursor^>
{
	bool operator() (SketchMusic::Cursor^ x, SketchMusic::Cursor^ y) const { return x->LT(y); }
	typedef SketchMusic::Cursor^ first_argument_type;
	typedef SketchMusic::Cursor^ second_argument_type;
	typedef bool result_type;
};
