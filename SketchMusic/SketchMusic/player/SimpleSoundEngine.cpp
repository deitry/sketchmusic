#include "pch.h"
#include "SimpleSoundEngine.h"
#include "SFData.h"
#include "Sample.h"
#include "Instrument.h"
#include "SoundFont.h"

/**
»нициализаци€ объекта SoundEngine, допускаетс€ только дл€ конкретного инструмента.
*/
SketchMusic::Player::SimpleSoundEngine::SimpleSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument)
{
	_pXAudio2 = pXAudio2;
	this->_instrument = instrument;

	// подгружаем семпл инструмента
	this->_sample = ref new SketchMusic::Sample(instrument->_name);
	_waveformat.wFormatTag = WAVE_FORMAT_PCM;
	_waveformat.nChannels = 1;
	_waveformat.nSamplesPerSec = 44100;
	_waveformat.nAvgBytesPerSec = 44100 * 2;
	_waveformat.nBlockAlign = 2;
	_waveformat.wBitsPerSample = 16;
	_waveformat.cbSize = 0;

	this->InitializeVoices();
}

SketchMusic::Player::SimpleSoundEngine::SimpleSoundEngine(SimpleSoundEngine^ engine)
{
	// копируем все пол€ как есть
	this->_pXAudio2 = engine->_pXAudio2;
	this->_instrument = engine->_instrument;
	this->_waveformat = engine->_waveformat;
	this->_sample = engine->_sample;
	
	this->InitializeVoices();

	// на вс€кий случай останавливаем
	this->Stop();
}

void SketchMusic::Player::SimpleSoundEngine::DestroyVoices()
{
	_available_voices = 0;
	_currentVoice = 0;

	// очищаем, если что-то есть. ”дал€ть сами голоса не надо, 
	// т.к. они и так все есть в _voices
	if (_available.size())
	{
		_available.clear();
	}

	// очищаем, если уже есть
	if (_voices.size())
	{
		for (auto voice : _voices)
		{
			voice->DestroyVoice();
		}
		_voices.clear();
	}
}

void SketchMusic::Player::SimpleSoundEngine::InitializeVoices()
{
	// удал€ем, если уже что-то есть
	DestroyVoices();

	// создаЄм несколько голосов дл€ данного "движка"
	for (int i = 0; i < MAX_VOICES; i++)
	{
		IXAudio2SourceVoice* pSourceVoice;
		HRESULT hr = _pXAudio2->CreateSourceVoice(&pSourceVoice, &_waveformat);
		this->_voices.push_back(pSourceVoice);
		this->_available.insert(std::make_pair(i, true));
		pSourceVoice->Start();
		pSourceVoice->SetVolume(0.3);
		_available_voices++;
	}
	_currentVoice = _available_voices - 1;
}

SketchMusic::Player::SimpleSoundEngine::~SimpleSoundEngine()
{
	DestroyVoices();
}

/**
«абирает из доступного пула голосов один и играет с его помощью ноту
*/
void SketchMusic::Player::SimpleSoundEngine::playNote(INote^ note, int duration, NoteOff^ noteOff)
{
	// по-старинке, без саундфонта
	IXAudio2SourceVoice* voice = this->GetVoice();
	if (voice == nullptr) 
		return;

	XAUDIO2_BUFFER _buffer; //= new XAUDIO2_BUFFER;
	_buffer.AudioBytes = _sample->getLength();
	_buffer.pAudioData = _sample->getSample()->Data;
	_buffer.Flags = XAUDIO2_END_OF_STREAM;
	_buffer.PlayBegin = 0;
	_buffer.PlayLength = 0;
	_buffer.LoopBegin = 0;
	_buffer.LoopLength = _sample->getLength() / 2;
	_buffer.pContext = nullptr;

	if (duration)
	{
		// TODO : умножить на коэффициент перевода лупа в тики (duration задаЄтс€ в тиках)
		// и частота, походу, тоже будет фигурировать
		_buffer.LoopCount = duration; // * _buffer.LoopLength / 44100
	}
	else
	{
		_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	float freq = _sample->getFrequency() * pow(2., note->_val / 12.);
	float origFreq = _sample->getFrequency();
	this->setFrequency(voice, freq, origFreq);

	// передаЄм буфер
	HRESULT hr = voice->SubmitSourceBuffer(&_buffer);
	if (FAILED(hr))
	{
		// TODO : заменить на throw Exception ?
	}
	voice->Start();

	auto stop = stopToken.get_token();
	auto release = concurrency::create_task([=]
	{
		unsigned int timeout = _buffer.LoopLength * _buffer.LoopCount * 1000. * origFreq / freq / 44100;
		// смотри выше про duration
		if (_buffer.LoopCount == XAUDIO2_LOOP_INFINITE)
		{
			timeout *= 100000;
		}
		concurrency::wait(timeout);
		this->ReleaseVoice(voice);
	}, stop);
}

/**
¬оспроизводит переданный набор нот
*/
void SketchMusic::Player::SimpleSoundEngine::Play(SketchMusic::INote^ note, int duration, NoteOff^ noteOff)
{
	// останавливаем все предыдущие воспроизведени€
	//this->Stop();

	// начинаем новое воспроизведение
	this->playNote(note, duration, noteOff);
}

/**
¬оспроизводит переданный набор нот
*/
void SketchMusic::Player::SimpleSoundEngine::Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes)
{
	// останавливаем все предыдущие воспроизведени€
	this->Stop();

	// начинаем новое воспроизведение
	for (auto note : notes)
	{
		this->playNote(note);
	}
}

void SketchMusic::Player::SimpleSoundEngine::Stop()
{
	stopToken.cancel();

	for (auto voice : _voices)
	{
		ReleaseVoice(voice);
		// одновременно останавливаем и возвращаем в пул
	}
	_available_voices = MAX_VOICES;
}

void SketchMusic::Player::SimpleSoundEngine::stopVoice(IXAudio2SourceVoice* voice)
{
	// останавливаем
	voice->ExitLoop();
	voice->Stop();

	XAUDIO2_BUFFER _buffer; //= new XAUDIO2_BUFFER;
	_buffer.AudioBytes = 0;
	_buffer.pAudioData = nullptr;
	_buffer.Flags = XAUDIO2_END_OF_STREAM;
	_buffer.PlayBegin = 0;
	_buffer.PlayLength = 0;
	_buffer.LoopBegin = 0;
	_buffer.LoopLength = 0;
	_buffer.pContext = nullptr;
	HRESULT hr = voice->SubmitSourceBuffer(&_buffer);

	hr = voice->FlushSourceBuffers();
}

void SketchMusic::Player::SimpleSoundEngine::setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq)
{
	if (voice)
	{
		voice->SetFrequencyRatio(freq / origFreq);
	}
}

IXAudio2SourceVoice* SketchMusic::Player::SimpleSoundEngine::GetVoice()
{
	int voiceNo = -1;

	for (auto&& it : _available)
	{
		if (it.second == true)
		{
			voiceNo = it.first;
			it.second = false;
			break;
		}
	}

	if (voiceNo != -1)
		return _voices[voiceNo];
	else
		return nullptr;
}

void SketchMusic::Player::SimpleSoundEngine::ReleaseVoice(IXAudio2SourceVoice* voice)
{
	int max = _voices.size();
	int voiceNo = -1;

	for (int i = 0; i < max; i++)
	{
		// находим номер голоса в основном списке
		if (_voices[i] == voice)
		{
			voiceNo = i;
			break;
		}
	}

	// голос в списке не обнаружен
	if (voiceNo == -1)
		return;

	auto v = _available.find(voiceNo);
	if (v != _available.end())
	{
		stopVoice(voice);
		v->second = true;
	}
}