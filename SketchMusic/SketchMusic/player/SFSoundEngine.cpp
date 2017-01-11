#include "pch.h"
#include "SFSoundEngine.h"
#include "SFData.h"
#include "Sample.h"
#include "Instrument.h"
#include "SoundFont.h"

/**
Инициализация объекта SoundEngine, допускается только для конкретного инструмента.
*/
SketchMusic::Player::SFSoundEngine::SFSoundEngine(Microsoft::WRL::ComPtr<IXAudio2> pXAudio2, SketchMusic::Instrument^ instrument)
{
	_pXAudio2 = pXAudio2;

	/*XAUDIO2_DEBUG_CONFIGURATION debug;
	debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	debug.LogFunctionName = 1;
	this->_pXAudio2->SetDebugConfiguration(&debug, nullptr);
	*/
	this->_instrument = instrument;
		
	auto data = SketchMusic::SFReader::SFData::ReadSFData(instrument->_name);
	if (data == nullptr)
		throw ref new Exception(0, "Не удалось создать объект");

	this->_sData = data->sdta;
	this->_soundFontData = data;
	this->SetPreset(instrument->preset);

	// это не очень правильно, но предполагаем, что все семплы в файле будут одного формата
	auto pZone = _sfPreset->zones->GetAt(0);
	auto zSample = pZone->instrument->zones->GetAt(0)->sample;
	if (zSample)
	{
		// не уверен насчёт корректности байт пер пер сек
		int chan = (zSample->sampleType == 1) ? 1 : 2;

		waveformat.wFormatTag = WAVE_FORMAT_PCM;
		waveformat.nChannels = 1;
		waveformat.nSamplesPerSec = zSample->sampleRate;
		waveformat.nAvgBytesPerSec = zSample->sampleRate*2;
		waveformat.nBlockAlign = 2;
		waveformat.wBitsPerSample = 2 * 8;
		waveformat.cbSize = 0;
	}

	this->InitializeVoices();
	stopToken = new concurrency::cancellation_token_source;
	locked = false;
}

SketchMusic::Player::SFSoundEngine::SFSoundEngine(SFSoundEngine^ engine)
{
	// копируем все поля как есть
	this->_pXAudio2 = engine->_pXAudio2;
	this->_instrument = engine->_instrument;
	this->waveformat = engine->waveformat;
	this->_sfPreset = engine->_sfPreset;
	this->_sData = engine->_sData;
	this->_soundFontData = engine->_soundFontData;
	
	this->InitializeVoices();
	this->stopToken = new concurrency::cancellation_token_source;

	// на всякий случай останавливаем
	this->Stop();
}

void SketchMusic::Player::SFSoundEngine::DestroyVoices()
{
	_available_voices = 0;
	_currentVoice = 0;

	// очищаем, если что-то есть. Удалять сами голоса не надо, 
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

void SketchMusic::Player::SFSoundEngine::InitializeVoices()
{
	// удаляем, если уже что-то есть
	DestroyVoices();

	// создаём несколько голосов для данного "движка"
	for (int i = 0; i < MAX_VOICES; i++)
	{
		IXAudio2SourceVoice* pSourceVoice;
		HRESULT hr = _pXAudio2->CreateSourceVoice(&pSourceVoice, &waveformat,0U,8.0F);
		this->_voices.push_back(pSourceVoice);
		this->_available.insert(std::make_pair(i, true));
		pSourceVoice->SetVolume(0.3);
		pSourceVoice->Start();
		_available_voices++;
	}
	_currentVoice = _available_voices - 1;
}

SketchMusic::Player::SFSoundEngine::~SFSoundEngine()
{
	DestroyVoices();
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
		for (auto pZone : this->_sfPreset->zones)
		{
			if (!(pZone->inZone(note->_val + MIDIKEYTOA4, 100))) continue;
			// обработка генераторов pZone?

			for (auto iZone : pZone->instrument->zones)
			{
				if (!(iZone->sample)) continue;
				if (!(iZone->inZone(note->_val + MIDIKEYTOA4, 100))) continue;

				IXAudio2SourceVoice* voice = this->GetVoice();
				if (voice == nullptr) return;

				XAUDIO2_BUFFER _buffer = { 0 };

				auto smpl = iZone->sample;

				// Формируем буфер для отправки
				// - загружаем данные по семплу
				_buffer.pAudioData = _sData->begin();
				_buffer.AudioBytes = _sData->Length;
				_buffer.Flags = XAUDIO2_END_OF_STREAM;
				_buffer.PlayBegin = smpl->start;
				_buffer.PlayLength = smpl->end;

				// если окажется, что оно нам не надо, то выкинем потом
				_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
				_buffer.LoopBegin = smpl->loopStart;
				_buffer.LoopLength = smpl->loopEnd;
				
				// 440 = midi 69 = SNote 0
				float orig = smpl->origPitch;
				double origFreq = 440 * pow(2., (orig - MIDIKEYTOA4) / 12.);
				double freq = 440 * pow(2., (note->_val) / 12.);
				
				// генераторы
				for (auto iter = iZone->generators.begin(); iter != iZone->generators.end(); iter++)
				{
					switch (iter->first)
					{
					case SFGeneratorID::pan:
						// range checking is done in the fluid_pan function 
						/*voice->pan = _GEN(voice, GEN_PAN);
						voice->amp_left = fluid_pan(voice->pan, 1) * voice->synth_gain / 32768.0f;
						voice->amp_right = fluid_pan(voice->pan, 0) * voice->synth_gain / 32768.0f;
						UPDATE_RVOICE_BUFFERS2(fluid_rvoice_buffers_set_amp, 0, voice->amp_left);
						UPDATE_RVOICE_BUFFERS2(fluid_rvoice_buffers_set_amp, 1, voice->amp_right);
						*/
						break;

					case SFGeneratorID::initialAttenuation:
						//voice->attenuation = ((fluid_real_t)(voice)->gen[GEN_ATTENUATION].val*ALT_ATTENUATION_SCALE) +
						//	(fluid_real_t)(voice)->gen[GEN_ATTENUATION].mod + (fluid_real_t)(voice)->gen[GEN_ATTENUATION].nrpn;

						// Range: SF2.01 section 8.1.3 # 48
						// Motivation for range checking:
						// OHPiano.SF2 sets initial attenuation to a whooping -96 dB
						//fluid_clip(voice->attenuation, 0.0, 1440.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_attenuation, voice->attenuation);
						break;

						// The pitch is calculated from three different generators.
						// Read comment in fluidsynth.h about GEN_PITCH.

						//case SFGeneratorID::pitch: // используетя в fluid под номером 59 ...
					case SFGeneratorID::coarseTune:
						freq *= pow(2., (iter->second->val.sword) / 12.);
						break;
					case SFGeneratorID::fineTune:
						freq *= pow(2., (iter->second->val.sword/100) / 12.); // ???
						
						// The testing for allowed range is done in 'fluid_ct2hz'
						//voice->pitch = (_GEN(voice, GEN_PITCH)
						//	+ 100.0f * _GEN(voice, GEN_COARSETUNE)
						//	+ _GEN(voice, GEN_FINETUNE));
						//UPDATE_RVOICE_R1(fluid_rvoice_set_pitch, voice->pitch);
						break;

					case SFGeneratorID::reverbEffectsSend:
						// The generator unit is 'tenths of a percent'.
						//voice->reverb_send = _GEN(voice, GEN_REVERBSEND) / 1000.0f;
						//fluid_clip(voice->reverb_send, 0.0, 1.0);
						//voice->amp_reverb = voice->reverb_send * voice->synth_gain / 32768.0f;
						//UPDATE_RVOICE_BUFFERS2(fluid_rvoice_buffers_set_amp, 2, voice->amp_reverb);
						break;

					case SFGeneratorID::chorusEffectsSend:
						/* The generator unit is 'tenths of a percent'. */
						//voice->chorus_send = _GEN(voice, GEN_CHORUSSEND) / 1000.0f;
						//fluid_clip(voice->chorus_send, 0.0, 1.0);
						//voice->amp_chorus = voice->chorus_send * voice->synth_gain / 32768.0f;
						//UPDATE_RVOICE_BUFFERS2(fluid_rvoice_buffers_set_amp, 3, voice->amp_chorus);
						break;

					case SFGeneratorID::overridingRootKey:
						// This is a non-realtime parameter. Therefore the .mod part of the generator
						// can be neglected.
						// NOTE: origpitch sets MIDI root note while pitchadj is a fine tuning amount
						// which offsets the original rate.  This means that the fine tuning is
						// inverted with respect to the root note (so subtract it, not add).

						origFreq = 440 * pow(2., (iter->second->val.sword - MIDIKEYTOA4) / 12.);

						//if (voice->gen[GEN_OVERRIDEROOTKEY].val > -1) {   //FIXME: use flag instead of -1
						//	voice->root_pitch = voice->gen[GEN_OVERRIDEROOTKEY].val * 100.0f
						//		- voice->sample->pitchadj;
						//}
						//else {
						//	voice->root_pitch = voice->sample->origpitch * 100.0f - voice->sample->pitchadj;
						//}
						//x = fluid_ct2hz(voice->root_pitch);
						//if (voice->sample != NULL) {
						//	x *= (fluid_real_t)voice->output_rate / voice->sample->samplerate;
						//}
						///* voice->pitch depends on voice->root_pitch, so calculate voice->pitch now */
						//fluid_voice_calculate_gen_pitch(voice);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_root_pitch_hz, x);

						break;

					case SFGeneratorID::initialFilterFc:
						// The resonance frequency is converted from absolute cents to
						// midicents .val and .mod are both used, this permits real-time
						// modulation.  The allowed range is tested in the 'fluid_ct2hz'
						// function [PH,20021214]

						//x = _GEN(voice, GEN_FILTERFC);
						//UPDATE_RVOICE_FILTER1(fluid_iir_filter_set_fres, x);
						break;

					case SFGeneratorID::initialFilterQ:
						// The generator contains 'centibels' (1/10 dB) => divide by 10 to
						// obtain dB

						//q_dB = _GEN(voice, GEN_FILTERQ) / 10.0f;

						// Range: SF2.01 section 8.1.3 # 8 (convert from cB to dB => /10)
						//fluid_clip(q_dB, 0.0f, 96.0f);

						// Short version: Modify the Q definition in a way, that a Q of 0
						// dB leads to no resonance hump in the freq. response.
						//
						// Long version: From SF2.01, page 39, item 9 (initialFilterQ):
						// "The gain at the cutoff frequency may be less than zero when
						// zero is specified".  Assume q_dB=0 / q_lin=1: If we would leave
						// q as it is, then this results in a 3 dB hump slightly below
						// fc. At fc, the gain is exactly the DC gain (0 dB).  What is
						// (probably) meant here is that the filter does not show a
						// resonance hump for q_dB=0. In this case, the corresponding
						// q_lin is 1/sqrt(2)=0.707.  The filter should have 3 dB of
						// attenuation at fc now.  In this case Q_dB is the height of the
						// resonance peak not over the DC gain, but over the frequency
						// response of a non-resonant filter.  This idea is implemented as
						// follows:
						//q_dB -= 3.01f;
						//UPDATE_RVOICE_FILTER1(fluid_iir_filter_set_q_dB, q_dB);

						break;

					case SFGeneratorID::modLfoToPitch:
						//x = _GEN(voice, GEN_MODLFOTOPITCH);
						//fluid_clip(x, -12000.0, 12000.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_pitch, x);
						break;

					case SFGeneratorID::modLfoToVolume:
						//x = _GEN(voice, GEN_MODLFOTOVOL);
						//fluid_clip(x, -960.0, 960.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_vol, x);
						break;

					case SFGeneratorID::modLfoToFilterFc:
						//x = _GEN(voice, GEN_MODLFOTOFILTERFC);
						//fluid_clip(x, -12000, 12000);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_fc, x);
						break;

					case SFGeneratorID::delayModLFO:
						//x = _GEN(voice, GEN_MODLFODELAY);
						//fluid_clip(x, -12000.0f, 5000.0f);
						//z = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
						//UPDATE_RVOICE_ENVLFO_I1(fluid_lfo_set_delay, modlfo, z);
						break;

					case SFGeneratorID::freqModLFO:
						// - the frequency is converted into a delta value, per buffer of FLUID_BUFSIZE samples
						// - the delay into a sample delay

						//x = _GEN(voice, GEN_MODLFOFREQ);
						//fluid_clip(x, -16000.0f, 4500.0f);
						//x = (4.0f * FLUID_BUFSIZE * fluid_act2hz(x) / voice->output_rate);
						//UPDATE_RVOICE_ENVLFO_R1(fluid_lfo_set_incr, modlfo, x);
						break;

					case SFGeneratorID::freqVibLFO:
						// vib lfo
						//
						// - the frequency is converted into a delta value, per buffer of FLUID_BUFSIZE samples
						// - the delay into a sample delay

						//x = _GEN(voice, GEN_VIBLFOFREQ);
						//fluid_clip(x, -16000.0f, 4500.0f);
						//x = 4.0f * FLUID_BUFSIZE * fluid_act2hz(x) / voice->output_rate;
						//UPDATE_RVOICE_ENVLFO_R1(fluid_lfo_set_incr, viblfo, x);
						break;

					case SFGeneratorID::delayVibLFO:
						//x = _GEN(voice, GEN_VIBLFODELAY);
						//fluid_clip(x, -12000.0f, 5000.0f);
						//z = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
						//UPDATE_RVOICE_ENVLFO_I1(fluid_lfo_set_delay, viblfo, z);
						break;

					case SFGeneratorID::vibLfoToPitch:
						//x = _GEN(voice, GEN_VIBLFOTOPITCH);
						//fluid_clip(x, -12000.0, 12000.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_viblfo_to_pitch, x);
						break;

					case SFGeneratorID::keynum:
						// GEN_KEYNUM: SF2.01 page 46, item 46
						// 
						// If this generator is active, it forces the key number to its
						// value.  Non-realtime controller.
						// 
						// There is a flag, which should indicate, whether a generator is
						// enabled or not.  But here we rely on the default value of -1.

						//x = _GEN(voice, GEN_KEYNUM);
						//if (x >= 0){
						//	voice->key = x;
						//}
						break;

					case SFGeneratorID::velocity:
						// GEN_VELOCITY: SF2.01 page 46, item 47
						//
						// If this generator is active, it forces the velocity to its
						// value. Non-realtime controller.
						//
						// There is a flag, which should indicate, whether a generator is
						// enabled or not. But here we rely on the default value of -1.
						//x = _GEN(voice, GEN_VELOCITY);
						//if (x > 0) {
						//	voice->vel = x;
						//}
						break;

					case SFGeneratorID::modEnvToPitch:
						//x = _GEN(voice, GEN_MODENVTOPITCH);
						//fluid_clip(x, -12000.0, 12000.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_modenv_to_pitch, x);
						break;

					case SFGeneratorID::modEnvToFilterFc:
						//x = _GEN(voice, GEN_MODENVTOFILTERFC);

						// Range: SF2.01 section 8.1.3 # 1
						// Motivation for range checking:
						// Filter is reported to make funny noises now and then

						//fluid_clip(x, -12000.0, 12000.0);
						//UPDATE_RVOICE_R1(fluid_rvoice_set_modenv_to_fc, x);
						break;


						// sample start and ends points
						//
						// Range checking is initiated via the
						// voice->check_sample_sanity flag,
						// because it is impossible to check here:
						// During the voice setup, all modulators are processed, while
						// the voice is inactive. Therefore, illegal settings may
						// occur during the setup (for example: First move the loop
						// end point ahead of the loop start point => invalid, then
						// move the loop start point forward => valid again.

					case SFGeneratorID::startAddrsOffset:              /* SF2.01 section 8.1.3 # 0 */
						_buffer.PlayBegin += iter->second->val.sword;
						
					case SFGeneratorID::startAddrsCoarseOffset:
						_buffer.PlayBegin += iter->second->val.sword * 32768;	
						break;
					case SFGeneratorID::endAddrsOffset:                 /* SF2.01 section 8.1.3 # 1 */
						_buffer.PlayLength += iter->second->val.sword;
						break;
					case SFGeneratorID::endAddrsCoarseOffset:           /* SF2.01 section 8.1.3 # 12 */
						_buffer.PlayLength += iter->second->val.sword * 32768;
						
						//if (voice->sample != NULL) {
						//	z = (voice->sample->end
						//		+ (int)_GEN(voice, GEN_ENDADDROFS)
						//		+ 32768 * (int)_GEN(voice, GEN_ENDADDRCOARSEOFS));
						//	UPDATE_RVOICE_I1(fluid_rvoice_set_end, z);
						//}
						//_buffer.LoopLength = iter->second->val.uword / 2;
						break;
					case SFGeneratorID::startloopAddrsOffset:           /* SF2.01 section 8.1.3 # 2 */
						_buffer.LoopBegin += iter->second->val.sword;
						break;
					case SFGeneratorID::startloopAddrsCoarseOffset:     /* SF2.01 section 8.1.3 # 45 */
						_buffer.LoopBegin += 32768*iter->second->val.sword;
						break;
					case SFGeneratorID::endloopAddrsOffset:
						_buffer.LoopLength += iter->second->val.sword;
						break;
					case SFGeneratorID::endloopAddrsCoarseOffset:
						_buffer.LoopLength += 32768*iter->second->val.sword;
						break;

						// Conversion functions differ in range limit
						//#define NUM_BUFFERS_DELAY(_v)   (unsigned int) (voice->output_rate * fluid_tc2sec_delay(_v) / FLUID_BUFSIZE)
						//#define NUM_BUFFERS_ATTACK(_v)  (unsigned int) (voice->output_rate * fluid_tc2sec_attack(_v) / FLUID_BUFSIZE)
						//#define NUM_BUFFERS_RELEASE(_v) (unsigned int) (voice->output_rate * fluid_tc2sec_release(_v) / FLUID_BUFSIZE)

						// volume envelope
						//
						// - delay and hold times are converted to absolute number of samples
						// - sustain is converted to its absolute value
						// - attack, decay and release are converted to their increment per sample
						//
					case SFGeneratorID::delayVolEnv:
						//x = _GEN(voice, GEN_VOLENVDELAY);
						//fluid_clip(x, -12000.0f, 5000.0f);
						//count = NUM_BUFFERS_DELAY(x);
						//fluid_voice_update_volenv(voice, FLUID_VOICE_ENVDELAY,
						//	count, 0.0f, 0.0f, -1.0f, 1.0f);
						break;

					case SFGeneratorID::attackVolEnv:
						//x = _GEN(voice, GEN_VOLENVATTACK);
						//fluid_clip(x, -12000.0f, 8000.0f);
						//count = 1 + NUM_BUFFERS_ATTACK(x);
						//fluid_voice_update_volenv(voice, FLUID_VOICE_ENVATTACK,
						//	count, 1.0f, count ? 1.0f / count : 0.0f, -1.0f, 1.0f);
						break;

					case SFGeneratorID::holdVolEnv:
					case SFGeneratorID::keynumToVolEnvHold:
						//count = calculate_hold_decay_buffers(voice, GEN_VOLENVHOLD, GEN_KEYTOVOLENVHOLD, 0); /* 0 means: hold */
						//fluid_voice_update_volenv(voice, FLUID_VOICE_ENVHOLD,
						//	count, 1.0f, 0.0f, -1.0f, 2.0f);
						break;

					case SFGeneratorID::decayVolEnv:
					case SFGeneratorID::sustainVolEnv:
					case SFGeneratorID::keynumToVolEnvDecay:
						//y = 1.0f - 0.001f * _GEN(voice, GEN_VOLENVSUSTAIN);
						//fluid_clip(y, 0.0f, 1.0f);
						//count = calculate_hold_decay_buffers(voice, GEN_VOLENVDECAY, GEN_KEYTOVOLENVDECAY, 1); // 1 for decay
						//fluid_voice_update_volenv(voice, FLUID_VOICE_ENVDECAY,
						//	count, 1.0f, count ? -1.0f / count : 0.0f, y, 2.0f);
						break;

					case SFGeneratorID::releaseVolEnv:
						//x = _GEN(voice, GEN_VOLENVRELEASE);
						//fluid_clip(x, FLUID_MIN_VOLENVRELEASE, 8000.0f);
						//count = 1 + NUM_BUFFERS_RELEASE(x);
						//fluid_voice_update_volenv(voice, FLUID_VOICE_ENVRELEASE,
						//	count, 1.0f, count ? -1.0f / count : 0.0f, 0.0f, 1.0f);
						break;

						// Modulation envelope
					case SFGeneratorID::delayModEnv:
						//x = _GEN(voice, GEN_MODENVDELAY);
						//fluid_clip(x, -12000.0f, 5000.0f);
						//fluid_voice_update_modenv(voice, FLUID_VOICE_ENVDELAY,
						//	NUM_BUFFERS_DELAY(x), 0.0f, 0.0f, -1.0f, 1.0f);
						break;

					case SFGeneratorID::attackModEnv:
						//x = _GEN(voice, GEN_MODENVATTACK);
						//fluid_clip(x, -12000.0f, 8000.0f);
						//count = 1 + NUM_BUFFERS_ATTACK(x);
						//fluid_voice_update_modenv(voice, FLUID_VOICE_ENVATTACK,
						//	count, 1.0f, count ? 1.0f / count : 0.0f, -1.0f, 1.0f);
						break;

					case SFGeneratorID::holdModEnv:
					case SFGeneratorID::keynumToModEnvHold:
						//count = calculate_hold_decay_buffers(voice, GEN_MODENVHOLD, GEN_KEYTOMODENVHOLD, 0); /* 1 means: hold */
						//fluid_voice_update_modenv(voice, FLUID_VOICE_ENVHOLD,
						//	count, 1.0f, 0.0f, -1.0f, 2.0f);
						break;

					case SFGeneratorID::decayModEnv:
					case SFGeneratorID::sustainModEnv:
					case SFGeneratorID::keynumToModEnvDecay:
						//count = calculate_hold_decay_buffers(voice, GEN_MODENVDECAY, GEN_KEYTOMODENVDECAY, 1); // 1 for decay
						//y = 1.0f - 0.001f * _GEN(voice, GEN_MODENVSUSTAIN);
						//fluid_clip(y, 0.0f, 1.0f);
						//fluid_voice_update_modenv(voice, FLUID_VOICE_ENVDECAY,
						//	count, 1.0f, count ? -1.0f / count : 0.0f, y, 2.0f);
						break;

					case SFGeneratorID::releaseModEnv:
						//x = _GEN(voice, GEN_MODENVRELEASE);
						//fluid_clip(x, -12000.0f, 8000.0f);
						//count = 1 + NUM_BUFFERS_RELEASE(x);
						//fluid_voice_update_modenv(voice, FLUID_VOICE_ENVRELEASE,
						//	count, 1.0f, count ? -1.0f / count : 0.0f, 0.0f, 2.0f);

						break;
					case SFGeneratorID::sampleModes:
						iZone->sampleMode = iter->second->val.uword;
						break;
					}
				}

				// модуляторы


				// финальная подготовка
				if (iZone->sampleMode == 0)
				{
					_buffer.LoopCount = 0;
					_buffer.LoopBegin = XAUDIO2_NO_LOOP_REGION;
					_buffer.LoopLength = 0;
				}
				this->setFrequency(voice, freq, origFreq);

				HRESULT hr = voice->SubmitSourceBuffer(&_buffer);
				if (FAILED(hr))
				{
					int i = 0;
					i++;
				}

				voice->Start();

				auto stop = (noteOff != nullptr) ? noteOff->GetToken() : stopToken->get_token();
				stop.register_callback([=]
				{
					auto release = concurrency::create_task([=]
					{		
						concurrency::wait(30);	// чтобы не сразу заканчивался - звучит неестественно
						this->ReleaseVoice(voice);
					});
				});

				// TODO : что вообще тут происходит?
				//auto cancelToken = new concurrency::cancellation_token_source;
				//auto tok = cancelToken->get_token();
				auto release = concurrency::create_task([=]
				{
					//unsigned int timeout = (double)_buffer.PlayLength / smpl->sampleRate * 1000. * origFreq / freq;
					unsigned int timeout = duration ? duration
						: 10000;	// на всякий случай
					concurrency::wait(timeout);
					// 161216 - не помню зачем это тут. Попробуем так
					//if (!concurrency::is_task_cancellation_requested())
					if (!stop.is_canceled())
					{
						this->ReleaseVoice(voice);
					}
				}, stop);
			}
		}
	}
}

/**
Воспроизводит переданный набор нот
*/
void SketchMusic::Player::SFSoundEngine::Play(SketchMusic::INote^ note, int duration, NoteOff^ noteOff)
{
	// останавливаем все предыдущие воспроизведения
	//this->Stop();

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

	/*for (auto voice : _voices)
	{
		ReleaseVoice(voice);
		// одновременно останавливаем и возвращаем в пул
	}*/
	_available_voices = MAX_VOICES;

	delete stopToken;
	// создаём новый источник отмены
	stopToken = new concurrency::cancellation_token_source;
}

void SketchMusic::Player::SFSoundEngine::stopVoice(IXAudio2SourceVoice* voice)
{
	// останавливаем
	HRESULT hr = voice->ExitLoop();
	hr = voice->Stop();

	XAUDIO2_BUFFER _buffer = { 0 }; //= new XAUDIO2_BUFFER;
	_buffer.AudioBytes = 0;
	_buffer.pAudioData = nullptr;
	_buffer.Flags = XAUDIO2_END_OF_STREAM;
	_buffer.pContext = nullptr;
	hr = voice->SubmitSourceBuffer(&_buffer);

	hr = voice->FlushSourceBuffers();
}

void SketchMusic::Player::SFSoundEngine::setFrequency(IXAudio2SourceVoice* voice, double freq, double origFreq)
{
	if (voice)
	{
		double ratio = freq / origFreq;
		voice->SetFrequencyRatio(ratio);
	}
}

IXAudio2SourceVoice* SketchMusic::Player::SFSoundEngine::GetVoice()
{
	while (locked);

	locked = true;

	if (_available.size() <= 0)
		return nullptr;
	/*{
	// если доступных голосов нет, возвращаем голос из общего пула
	if (_currentVoice < 0)
	{
	_currentVoice == _voices.size() - 1;
	}
	return _voices[_currentVoice--];
	}*/
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
	
	locked = false;
	if (voiceNo != -1)
		return _voices[voiceNo];
	else
		return nullptr;
}

void SketchMusic::Player::SFSoundEngine::ReleaseVoice(IXAudio2SourceVoice* voice)
{
	while (locked);
	
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

	locked = true;	// блокируем, чтобы у нас прям под носом не менялся размер
		// и содержание _available

	auto v = _available.find(voiceNo);
	if (v != _available.end())
	{
		stopVoice(voice);
		v->second = true;
	}

	locked = false;
}