#pragma once

#include "xaudio2.h"
#include "../SketchMusic.h"
#include "ISoundEngine.h"

using namespace SketchMusic;
using namespace SketchMusic::Player;

/**
����� ��������� �������� ���������� � ������������� ������������ �� �� ������������� � ������.

TODO : ����� ������ ��������� ������ ������������ ��������������� ������������� � ������������ 
������ ������� ������������ � ��������� � ������������ � �������� ���������?
*/
ref class SketchMusic::Player::SoundEnginePool sealed
{
private:
	std::list<ISoundEngine^> _engines;
	std::list<std::pair<ISoundEngine^, bool>> _available;		// ��� ����������� ��������� ������������
		// ! - ���������� ��������� ����� ������, ��� ���������� ����, ������ ��� ���� ����� ����������� �����
	
	Microsoft::WRL::ComPtr<IXAudio2> _xaudio2;	// ����� ��� ��������� �������� ������������
	ISoundEngine^ _default;	// �����, ������� ����� ����������� �� ���������

	ISoundEngine^ GetIfAvailable(Instrument^ instrument);
	~SoundEnginePool();
internal:
	SoundEnginePool(Microsoft::WRL::ComPtr<IXAudio2> xaudio2) { _xaudio2 = xaudio2; }

	ISoundEngine^ GetSoundEngine(Instrument^ instrument);
	void ReleaseSoundEngine(ISoundEngine^ engine);

	void AddSoundEngine(Instrument^ instrument);
	void DeleteSoundEngine(Instrument^ instrument);
};