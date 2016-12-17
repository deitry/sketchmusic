#pragma once

#include "../SketchMusic.h"
#include "ISoundEngine.h"
#include "SoundFont.h"
#include <xaudio2.h>

#define MIDIKEYTOA4 69

/**
Отвечает за звучание определённого инструмента.
*/
ref class SketchMusic::Player::SFSoundEngine sealed : ISoundEngine
{
private:
	static const int MAX_VOICES = 16;
	bool locked;

	SketchMusic::Instrument^ _instrument;
	std::vector<IXAudio2SourceVoice*> _voices;		// пул всех голосов
	std::map<int, bool> _available;	// пул свободных голосов - можно выкинуть
	int _available_voices;
	unsigned int _currentVoice;

	Microsoft::WRL::ComPtr<IXAudio2> _pXAudio2;
	WAVEFORMATEX waveformat;
	SFPreset^ _sfPreset;	// данные о семплах
		// TODO : оставить только отдельные пресеты, остаьльные данные игнорировать для оптимизации
	Platform::Array<unsigned char>^ _sData;

	concurrency::cancellation_token_source* stopToken;

	void playNote(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr);
		// TODO : ужесточить до SketchMusic::SNote ? Тогда Енжин сможет проигрывать только конкретизованные ноты
	void setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq);

	
	IXAudio2SourceVoice* GetVoice();
	void ReleaseVoice(IXAudio2SourceVoice* voice);
	void stopVoice(IXAudio2SourceVoice* voice);

	void InitializeVoices();
	void DestroyVoices();

	~SFSoundEngine();

internal:
	SFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument);
	SFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::SFReader::SFData^ soundFont);	// конструктор на основе уже созданного soundFont
	
	SFSoundEngine(SFSoundEngine^ engine);

public:
	virtual void Play(SketchMusic::INote^ note, int duration = 0, NoteOff^ noteOff = nullptr); // duration в тиках
	virtual void Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes);
	virtual void Stop();
	virtual SketchMusic::Instrument^ getInstrument() { return _instrument; }
	virtual ISoundEngine^ Copy()
	{
		return ref new SFSoundEngine(this);
	}
};