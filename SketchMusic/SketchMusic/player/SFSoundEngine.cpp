#include "pch.h"
#include "SFSoundEngine.h"
#include "SFData.h"
#include "Sample.h"
#include "Instrument.h"
#include "SoundFont.h"
#include "../base/SNoteEnd.h"

SFGenAmount^ GetAmount(std::map<SFGeneratorID, SFGenAmount^> generators, SFGeneratorID id)
{
	auto lookup = generators.find(id);
	if (lookup != generators.end())
	{
		return lookup->second;
	}
	return ref new SFGenAmount;
}

SketchMusic::Player::SFSoundEngine::SFSoundEngine()
{
	
}

/**
Инициализация объекта SoundEngine, допускается только для конкретного инструмента.
*/
SketchMusic::Player::SFSoundEngine^ SketchMusic::Player::SFSoundEngine::GetSFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument)
{
	auto data = SketchMusic::SFReader::SFData::ReadSFData(instrument->FileName);
	if (data == nullptr)
		return nullptr;
	
	auto engine = ref new SFSoundEngine();

	engine->_pXAudio2 = pXAudio2;
	engine->_instrument = instrument;
	engine->_sData = data->sdta;
	engine->_soundFontData = data;
	engine->SetPreset(instrument->preset);

	engine->stopToken = new cancellation_token_source();

	return engine;
}

SketchMusic::Player::SFSoundEngine::SFSoundEngine(SFSoundEngine^ engine)
{
	// копируем все поля как есть
	this->_pXAudio2 = engine->_pXAudio2;
	this->_instrument = engine->_instrument;
	this->_sfPreset = engine->_sfPreset;
	this->_sData = engine->_sData;
	this->_soundFontData = engine->_soundFontData;
	
	//this->InitializeVoices();
	this->stopToken = new cancellation_token_source();

	// на всякий случай останавливаем
	this->Stop();
}


IXAudio2SourceVoice* SketchMusic::Player::SFSoundEngine::InitializeVoice(SFSample^ sample)
{
	WAVEFORMATEX waveformat;
	waveformat.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.nChannels = 1;
	waveformat.nSamplesPerSec = sample->sampleRate;
	waveformat.nAvgBytesPerSec = sample->sampleRate * 2;
	waveformat.nBlockAlign = 2;
	waveformat.wBitsPerSample = 2 * 8;
	waveformat.cbSize = 0;

	IXAudio2SourceVoice* pSourceVoice;
	try
	{
		HRESULT hr = _pXAudio2->CreateSourceVoice(&pSourceVoice, &waveformat, 0U, 8.0F);
		pSourceVoice->Start();

		return pSourceVoice;
	}
	catch (...)
	{
		return nullptr;
	}
}

SketchMusic::Player::SFSoundEngine::~SFSoundEngine()
{
	stopToken->cancel();
	delete stopToken;
}

/**
Забирает из доступного пула голосов один и играет с его помощью ноту
*/
void SketchMusic::Player::SFSoundEngine::playNote(INote^ note, int duration, NoteOff^ noteOff)
{
	auto end = dynamic_cast<SNoteEnd^>(note);
	if (end) return;

	// - проходимся по всем зонам и т.д., вычисляем конечные параметры звука
	if (_sfPreset)
	{
		// параметры, общие для пресета
		double presetTuning = 0.; // в "центах" : 100 cents = 1 semitone
		double presetAttenuation = 0;	// в сентибеллах
		
		// сразу обрабатываем глобальную зону
		if (_sfPreset->globalZone)
		{
			auto gens = _sfPreset->globalZone->generators;
			
			presetTuning = GetAmount(gens, SFGeneratorID::coarseTune)->val.sword * 100. 
						 + GetAmount(gens, SFGeneratorID::fineTune)->val.sword;
			presetAttenuation = GetAmount(gens, SFGeneratorID::initialAttenuation)->val.sword;
		}

		for (auto&& pZone : this->_sfPreset->zones)
		{
			// тут уже пролистываем глобальную зону
			if (_sfPreset->globalZone == pZone || pZone->instrument == nullptr) continue;
			if (!(pZone->inZone(note->_val + MIDIKEYTOA4, 100))) continue;

			// параметры, общие для инструмента
			double instrTuning = 0.; // в "центах" : 100 cents = 1 semitone
			double instrAttenuation = 0;	// в сентибеллах

			// глобальная зона инструмента
			if (pZone->instrument->globalZone)
			{
				auto gens = pZone->instrument->globalZone->generators;

				instrTuning = GetAmount(gens, SFGeneratorID::coarseTune)->val.sword * 100.
					+ GetAmount(gens, SFGeneratorID::fineTune)->val.sword;
				instrAttenuation = GetAmount(gens, SFGeneratorID::initialAttenuation)->val.sword;
			}

			for (auto&& iZone : pZone->instrument->zones)
			{
				if (!(iZone->sample)) continue;
				if (pZone->instrument->globalZone == iZone) continue;

				if (!(iZone->inZone(note->_val + MIDIKEYTOA4, 100))) continue;

				// получаем голос
				IXAudio2SourceVoice* voice = this->InitializeVoice(iZone->sample);
				if (voice == nullptr) return;

				XAUDIO2_BUFFER _buffer = { 0 };
				auto smpl = iZone->sample;

				// Формируем буфер для отправки
				// - загружаем данные по семплу
				_buffer.pAudioData = _sData->begin();
				_buffer.AudioBytes = _sData->Length;
				_buffer.Flags = XAUDIO2_END_OF_STREAM;
				_buffer.PlayBegin = smpl->start 
								  + GetAmount(iZone->generators, SFGeneratorID::startAddrsOffset)->val.sword
								  + GetAmount(iZone->generators, SFGeneratorID::startAddrsCoarseOffset)->val.sword * 32768;
				_buffer.PlayLength = smpl->end
					+ GetAmount(iZone->generators, SFGeneratorID::endAddrsOffset)->val.sword
					+ GetAmount(iZone->generators, SFGeneratorID::endAddrsCoarseOffset)->val.sword * 32768;

				if (GetAmount(iZone->generators, SFGeneratorID::sampleModes)->val.sword == 0)
				{
					_buffer.LoopCount = 0;
					_buffer.LoopBegin = XAUDIO2_NO_LOOP_REGION;
					_buffer.LoopLength = 0;
				}
				else
				{
					// если окажется, что оно нам не надо, то выкинем потом
					_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
					_buffer.LoopBegin = smpl->loopStart
						+ GetAmount(iZone->generators, SFGeneratorID::startloopAddrsOffset)->val.sword
						+ GetAmount(iZone->generators, SFGeneratorID::startloopAddrsCoarseOffset)->val.sword * 32768;
					_buffer.LoopLength = smpl->loopEnd
						+ GetAmount(iZone->generators, SFGeneratorID::endloopAddrsOffset)->val.sword
						+ GetAmount(iZone->generators, SFGeneratorID::endloopAddrsCoarseOffset)->val.sword * 32768;
				}

				// 440 = midi 69 = SNote 0
				double orig = GetAmount(iZone->generators, SFGeneratorID::overridingRootKey)->val.sword;
				if (orig == 0 || orig == -1) orig = smpl->origPitch;

				double origFreq = 440 * pow(2., (orig - MIDIKEYTOA4) / 12.);
				double freq = 440 * pow(2., (note->_val
											+ GetAmount(iZone->generators, SFGeneratorID::coarseTune)->val.sword
											+ GetAmount(iZone->generators, SFGeneratorID::fineTune)->val.sword / 100
											+ iZone->sample->pitchAdj / 100) / 12.);

				// TODO : когда-нибудь : модуляторы				

				// - окончательно применяем частоту
				this->setFrequency(voice, freq, origFreq);
				// - аттенюейшн / громкость
				voice->SetVolume(0.2f);
				// TODO : - панорамирование
				// TODO : - эффекты
				
				HRESULT hr = voice->SubmitSourceBuffer(&_buffer);
				if (FAILED(hr))
				{
					int i = 0;
					i++;
				}

				voice->Start();

				auto stop = (noteOff != nullptr) ? noteOff->GetToken() : stopToken->get_token();
				auto regToken = stop.register_callback([=]
				{
					auto release = concurrency::create_task([=]
					{		
						concurrency::wait(30);	// чтобы не сразу заканчивался - звучит неестественно
						this->ReleaseVoice(voice);
					});
				});

				if (duration)
				{
					auto release = concurrency::create_task([=]
					{
						concurrency::wait(duration);
						if (!stop.is_canceled())
						{
							this->ReleaseVoice(voice);
							stop.deregister_callback(regToken);
						}
					}, stop);
				}
			}
		}
	}
}

/**
Воспроизводит переданную ноту
*/
void SketchMusic::Player::SFSoundEngine::Play(SketchMusic::INote^ note, int duration, NoteOff^ noteOff)
{
	if (!note) return;

	// останавливаем все предыдущие воспроизведения
	this->Stop();

	// начинаем новое воспроизведение
	this->playNote(note, duration, noteOff);
}

/**
Воспроизводит переданный набор нот
*/
void SketchMusic::Player::SFSoundEngine::Play(Windows::Foundation::Collections::IVector<SketchMusic::INote^>^ notes)
{
	// останавливаем все предыдущие воспроизведения
	this->Stop();
	
	// начинаем новое воспроизведение
	for (auto note : notes)
	{
		this->playNote(note);
	}
}

void SketchMusic::Player::SFSoundEngine::Stop()
{
	stopToken->cancel();
	delete stopToken;
	stopToken = new cancellation_token_source();
}

// попользовались - избавились
void SketchMusic::Player::SFSoundEngine::ReleaseVoice(IXAudio2SourceVoice* voice)
{
	if (voice == nullptr) return;
	// останавливаем
	HRESULT hr = voice->ExitLoop();
	hr = voice->Stop();

	voice->DestroyVoice();
}

void SketchMusic::Player::SFSoundEngine::setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq)
{
	if (voice)
	{
		auto ratio = freq / origFreq;
		voice->SetFrequencyRatio(static_cast<float>(ratio));
	}
}
