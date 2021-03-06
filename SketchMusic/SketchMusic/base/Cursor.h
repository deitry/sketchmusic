#pragma once

#include "../SketchMusic.h"

using namespace Windows::Data::Json;

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
[Windows::UI::Xaml::Data::Bindable]
public ref class SketchMusic::Cursor sealed
{

	int _beat;	// ����� "��������" ����.
				// ��������� �� ����� ���������� � ������������
				// � ���������� ���������.
				// ���� ���������� ����� ���������� ������ ������������ ������� �����.
	float _tick;	// ����� "�������" ����. ������ �������, ��� ������� �����,
					// ���� ����� �������, ��� � ����� ������� ���� TICK_IN_BEAT ������
					// UPD: �������� � int �� float ��� ������� ��������� ������� � ����.
					// ������ �������������� ����� �����: ��� �������� ������� � ������ -���� ����� �����
					// ������� ����, ������� ����� (������ = �����, ���������, ������������ ���������� �� ��������� ���� � �.�.)
					// � ������ ��������� �������������� ���������� ���������� ���������.

public:	
	// ������������
	Cursor() {_beat = 0; _tick = 0; }
	[Windows::Foundation::Metadata::DefaultOverloadAttribute]
	Cursor(int b) { _beat = b; _tick = 0; }
	Cursor(int b, float t);
	Cursor(Cursor^ cur) { if (cur) { _beat = cur->_beat; _tick = cur->_tick; } else { _beat = 0; _tick = 0; } }

	property int Beat
	{ int get() { return _beat; }
	void set(int a) { _beat = a; }}
	
	property float Tick
	{ float get() { return _tick; }
	void set(float a) { _tick = a; }}

	// ������ � �����
	
	void moveTo(SketchMusic::Cursor^ pos) { _beat = pos->Beat; _tick = pos->Tick; }
	void setPos(int b, float t) { _beat = b; _tick = t; }
	void setPos(int b) { setPos(b, 0); }

	float toTicks() { return _tick + _beat*SketchMusic::TICK_IN_BEAT; }

	// ��������

	SketchMusic::Cursor^ inc();
	SketchMusic::Cursor^ inc(SketchMusic::Cursor^ offset);

	void incTick(float inc)
	{
		if (inc == 0) return;

		this->_tick += inc;
		while (this->_tick > TICK_IN_BEAT)
		{
			this->_tick -= TICK_IN_BEAT;
			this->_beat += 1;
		}
	}

	SketchMusic::Cursor^ incBeat()
	{
		this->_beat++;
		this->_tick = 0;
		return ref new SketchMusic::Cursor(_beat,0);
	}
	SketchMusic::Cursor^ dec();
	SketchMusic::Cursor^ dec(SketchMusic::Cursor^ offset);

	SketchMusic::Cursor^ decBeat()
	{
		if (this->_beat > 0)
			this->_beat--;
		this->_tick = 0;
		return ref new SketchMusic::Cursor(_beat, 0);
	}
	
	void decTick(float dec)
	{
		if (dec == 0) return;

		this->_beat = static_cast<int>(dec / TICK_IN_BEAT);
		this->_tick -= static_cast<int>(std::floor(dec)) % TICK_IN_BEAT;
		if (this->_tick < 0)
		{
			if (this->_beat > 0)
			{
				--this->_beat;
				this->_tick = TICK_IN_BEAT - dec;
			}
			else
			{
				this->_tick = 0;
			}
		}
	}
	
	bool EQ(SketchMusic::Cursor^ that);
	bool GT(SketchMusic::Cursor^ that);
	bool LT(SketchMusic::Cursor^ that);
	bool LE(SketchMusic::Cursor^ that);

	Platform::String^ ToString() override { return "" + _beat + ":" + _tick; }
	IJsonValue^ Serialize();
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
