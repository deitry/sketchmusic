#pragma once

#include "../SketchMusic.h"
#include "xaudio2.h"

/**
���������� ��������� �����.
�� ����� ��������� � �����.
*/
public ref class SketchMusic::Sample sealed
{
private:
	Platform::String^ _name;	// ������������
	int _length;				// ����� ������ - ����� ���� ���������? ������ �� ������ � �������� ������� �������������
	float _frequency;			// ������ ���� - �������, ��������������� ������� ������
	Platform::Array<uint8>^ _sample;	// ���������� ��� �����
	~Sample();

public:
	Sample(Platform::String^ FileName);	// ���������� ����� �� �����
		// - ��� �� �������� ������?

	Platform::Array<uint8>^ getSample() { return _sample; }
	int getLength() { return _length; }
	float getFrequency() { return _frequency; }
	Platform::String^ getName() { return _name; }
};