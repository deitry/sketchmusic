#pragma once

#include "../SketchMusic.h"
#include "ISoundEngine.h"
#include "SoundFont.h"
#include <xaudio2.h>

/**
�������� �� �������� ������������ �����������.
*/
ref class SketchMusic::Player::SimpleSoundEngine sealed : ISoundEngine
{
private:
	static const int MAX_VOICES = 16;

	SketchMusic::Instrument^ _instrument;
	std::vector<IXAudio2SourceVoice*> _voices;	// ��� ��������� �������
	std::map<int, bool> _available;	// ��� ��������� �������
	int _available_voices;
	unsigned int _currentVoice;

	Microsoft::WRL::ComPtr<IXAudio2> _pXAudio2;
	WAVEFORMATEX _waveformat;
	Sample^ _sample;			// ����� - ���� �� ��������� �� �� ����������
	
	concurrency::cancellation_token_source* stopToken;

	void playNote(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr);
	// TODO : ���������� �� SketchMusic::SNote ? ����� ����� ������ ����������� ������ ���������������� ����
	void setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq);


	IXAudio2SourceVoice* GetVoice();
	void ReleaseVoice(IXAudio2SourceVoice* voice);
	void stopVoice(IXAudio2SourceVoice* voice);

	void InitializeVoices();
	void DestroyVoices();

	~SimpleSoundEngine();

internal:
	SimpleSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument);
	SimpleSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::SFReader::SFData^ soundFont);	// ����������� �� ������ ��� ���������� soundFont

	SimpleSoundEngine(SimpleSoundEngine^ engine);

public:
	virtual void Play(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr); // duration � �����
	virtual void Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes);
	virtual void Stop();
	virtual SketchMusic::Instrument^ getInstrument() { return _instrument; }
	virtual ISoundEngine^ Copy()
	{
		return ref new SimpleSoundEngine(this);
	}
	virtual void SetPreset(Platform::String^ preset) {}
};