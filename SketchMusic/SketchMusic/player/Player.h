#pragma once
#include "../SketchMusic.h"
#include "../base/Composition.h"
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
	SNote^ metroNote;

	void playMetronome();
	SketchMusic::Player::PlayerState m_state;
	float m_bpm;

	INote^ concretize(INote^ note, Text^ currentText);
	SNote^ concretizeRNote(SRNote^ rnote, SHarmony^ localHarmony);
	SNote^ concretizeGNote(SGNote^ gnote);

	void setDefaultRelatives();

	std::vector<SHarmony^> localHarmonyVect;

public:
	event EventHandler<SketchMusic::Player::PlayerState>^ StateChanged;
	event EventHandler<float>^ BpmChanged;
	event EventHandler<SketchMusic::Cursor^>^ CursorPosChanged;

	property float quantize; // 0 - ������ ��� ����, >0 - ��������� ����������� (TickInBeat/2 ...)
		// �������� ����� ���������������� � ��������� TextRow
	
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
	
	property bool needPlayGeneric;
	property bool cycling;
	property bool StopAtLast;
	property int precount;
	property bool recording;	// ��� ���������� ���������, ����� ������ ���

	property float _BPM
	{
		float get() { return m_bpm; }
		void set(float _bpm)
		{
			if (m_bpm != _bpm)
			{
				m_bpm = _bpm; BpmChanged(this, m_bpm);
			}
		}
	}

	property SScale^ _scale;
	property SHarmony^ _harmony;
	property SHarmony^ _localHarmony; // ��� "���������" �������� ����� �������������� ��� ��������������� ����� ����
	property bool needMetronome;

	property SketchMusic::Cursor^ _cursor;

	Player();
	
	void playText(SketchMusic::CompositionData^ text, SketchMusic::CompositionLibrary^ lib, SketchMusic::Cursor^ start, Text^ currentText);
	void playText(SketchMusic::CompositionData^ text, SketchMusic::Cursor^ start, Text^ currentText); //, SketchMusic::Cursor^ end
	void playSingleNote(SketchMusic::INote^ note, SketchMusic::Text^ currentText, int duration, SketchMusic::Player::NoteOff^ stop); // ��� ������������ ���� � ����������

	void stop();
	void stopKeyboard();
	
	SketchMusic::SFReader::SFData^ GetSFData(SketchMusic::Instrument^ instrument);	// TODO : soundpool ������ (?) ���� �������� �� ������
		// - ����� ����� ����������� "����������" ������������ - ��������, ��������� ���������� ������� ����

	SketchMusic::Cursor^ getCursor() { return this->_cursor; }
	void actualizeControlData(SketchMusic::CompositionData^ data,
							  SketchMusic::Text^ currentText,
							  SketchMusic::Cursor^ pos);
};
