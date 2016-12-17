#pragma once

#include "../SketchMusic.h"
#include "ISoundEngine.h"
#include "SoundFont.h"
#include <xaudio2.h>

#define MIDIKEYTOA4 69

/**
�������� �� �������� ������������ �����������.
*/
ref class SketchMusic::Player::SFSoundEngine sealed : ISoundEngine
{
private:
	static const int MAX_VOICES = 16;
	bool locked;

	SketchMusic::Instrument^ _instrument;
	std::vector<IXAudio2SourceVoice*> _voices;		// ��� ���� �������
	std::map<int, bool> _available;	// ��� ��������� ������� - ����� ��������
	int _available_voices;
	unsigned int _currentVoice;

	Microsoft::WRL::ComPtr<IXAudio2> _pXAudio2;
	WAVEFORMATEX waveformat;
	SFPreset^ _sfPreset;	// ������ � �������
		// TODO : �������� ������ ��������� �������, ���������� ������ ������������ ��� �����������
	Platform::Array<unsigned char>^ _sData;

	concurrency::cancellation_token_source* stopToken;

	void playNote(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr);
		// TODO : ���������� �� SketchMusic::SNote ? ����� ����� ������ ����������� ������ ���������������� ����
	void setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq);

	
	IXAudio2SourceVoice* GetVoice();
	void ReleaseVoice(IXAudio2SourceVoice* voice);
	void stopVoice(IXAudio2SourceVoice* voice);

	void InitializeVoices();
	void DestroyVoices();

	~SFSoundEngine();

internal:
	SFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument);
	SFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::SFReader::SFData^ soundFont);	// ����������� �� ������ ��� ���������� soundFont
	
	SFSoundEngine(SFSoundEngine^ engine);

public:
	virtual void Play(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr); // duration � �����
	virtual void Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes);
	virtual void Stop();
	virtual SketchMusic::Instrument^ getInstrument() { return _instrument; }
	virtual ISoundEngine^ Copy()
	{
		return ref new SFSoundEngine(this);
	}
};