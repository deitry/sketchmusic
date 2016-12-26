#pragma once
#include "../SketchMusic.h"
#include <xaudio2.h>

public ref class SketchMusic::Player::Player sealed
{
private:
	// ��������� �����
	// ? TODO : ������� ���������� xaudio ����������� � SoundEnginePool
	Microsoft::WRL::ComPtr<IXAudio2> pXAudio2;
	IXAudio2MasteringVoice * pMasteringVoice;	
	SoundEnginePool^ _engines;

	ISoundEngine^ _keyboardEngine;	// ������ ����� ���������� ������ ��� ������������ ��� � ����������
	ISoundEngine^ _metronome;

	void playMetronome();
	
public:
	property int _state;
	property bool cycling;
	property float _BPM;
	property bool needMetronome;

	property SketchMusic::Cursor^ _cursor;

	Player();
	
	void playText(Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ text, SketchMusic::Cursor^ start);
	void playSingleNote(SketchMusic::INote^ note, SketchMusic::Instrument^ instrument, int duration, SketchMusic::Player::NoteOff^ stop); // ��� ������������ ���� � ����������

	void stop();
	void stopKeyboard();

	SketchMusic::Cursor^ getCursor() { return this->_cursor; }
};



