#pragma once

#include "../SketchMusic.h"

/**
�������� �� �������� ������������ �����������.
*/
interface class SketchMusic::Player::ISoundEngine
{
public:

	ISoundEngine^ Copy();

	void Play(SketchMusic::INote^ note, int duration, NoteOff^ noteOff); // duration � �����
		// ��������, ����� ��������� �� ��� ��������� �������

	// ����� ��� ������������ ��������� ���, �������� ��� ��������� ��� ����� � ����������
	void Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes);
		// ����� ��� ������������ ����� ���������� ���, ��������, ��� ��������������� ������
	void Stop();
	SketchMusic::Instrument^ getInstrument();
	void SetPreset(Platform::String^ preset);
	//void setEffect();

	// ������� �� ������ � ������ �����? ��������� �����, ��������� � �.�. - ��������� ������
};