#pragma once

#include "../SketchMusic.h"

using namespace SketchMusic;
using namespace SketchMusic::View;

public ref class SketchMusic::View::Key sealed
{
public:
	property KeyType originalType;	// � ���������� ��� � �������� ����� ����������
	property int originalValue;		// ����� ��������� ��, ��� ���� ���� ��� �������� - ������� � �������
	property KeyType type;
	property float value;
	property int shift;	// ������ ��������� �������� ��� ������� �����
	property Object^ parent;	// ? ������ �� ctrl, ������� ��������� ������������ ������

	Key(int keyNum)
	{
		// ��������� �� ����� � ������� �������� "���������"
		// ��������� ��� ����� ���� ��� ��������
		// �������� ���������� ���
		// int = TTNNN
		// ��� �������� � �������� �������������� KeyType
		type = (KeyType)abs(keyNum / 1000);
		originalType = type;
		value = keyNum % 1000;
		originalValue = value;
		shift = 0;
	}

	Key(KeyType type, float value)
	{
		this->type = type;
		this->value = value;
		this->shift = 0;
	}
};

template<>
struct std::less<SketchMusic::View::Key^>
{
	bool operator() (SketchMusic::View::Key^ x, SketchMusic::View::Key^ y) const 
	{ 
		return x->type < y->type; 
	
	}
	typedef SketchMusic::View::Key^ first_argument_type;
	typedef SketchMusic::View::Key^ second_argument_type;
	typedef bool result_type;
};