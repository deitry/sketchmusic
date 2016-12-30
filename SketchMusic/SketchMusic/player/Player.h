#pragma once
#include "../SketchMusic.h"
#include <xaudio2.h>

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::Player::Player sealed //: INotifyPropertyChanged
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
	SketchMusic::Player::PlayerState m_state;
public:
	event EventHandler<SketchMusic::Player::PlayerState>^ StateChanged;
	
	property SketchMusic::Player::PlayerState _state
	{
		SketchMusic::Player::PlayerState get() { return m_state; }
		void set(SketchMusic::Player::PlayerState _state)
		{
			if (m_state != _state)
			{
				m_state = _state; StateChanged(this, _state);
			}
		}
	}
	
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



