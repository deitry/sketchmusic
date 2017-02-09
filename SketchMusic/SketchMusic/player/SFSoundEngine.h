#pragma once

#include "../SketchMusic.h"
#include "ISoundEngine.h"
#include "SoundFont.h"
#include <xaudio2.h>
#include "SFData.h"

/**
�������� �� �������� ������������ �����������.
*/
ref class SketchMusic::Player::SFSoundEngine sealed : ISoundEngine
{
private:
	static const int MAX_VOICES = 16;

	SketchMusic::Instrument^ _instrument;
	unsigned int _currentVoice;

	Microsoft::WRL::ComPtr<IXAudio2> _pXAudio2;
	SFPreset^ _sfPreset;	// ������ � �������
	Platform::Array<unsigned char>^ _sData;

	concurrency::cancellation_token_source* stopToken;

	void playNote(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr);
	void setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq);
		
	IXAudio2SourceVoice* InitializeVoice(SFSample^ sample);
	void ReleaseVoice(IXAudio2SourceVoice* voice);

	SFSoundEngine();
	~SFSoundEngine();

internal:
	static SFSoundEngine^ GetSFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument);
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

	virtual void SetPreset(Platform::String^ preset)
	{
		if (_soundFontData)
		{
			if (preset == nullptr || preset == "")
			{
				_sfPreset = this->_soundFontData->presets->GetAt(0);
				_sfPreset->updateParams();
				return;
			}

			for (auto pset : _soundFontData->presets)
			{
				if (pset->name == preset)
				{
					_sfPreset = pset;
					_sfPreset->updateParams();
					return;
				}
			}
		}
	}

	property SFData^ _soundFontData;
};