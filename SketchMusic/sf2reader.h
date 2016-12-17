#pragma once

// ÚËÔ˚ ‰‡ÌÌ˚ı ÔÓ ÒÔÂˆËÙËÍ‡ˆËË SoundFont
typedef uint8 BYTE;
typedef uint16 SHORT;

typedef uint8 CHAR;
typedef uint16 WORD;
typedef uint32 DWORD;

namespace SF2READER
{
	/*<SFBK - form> -> RIFF
		(ësfbkí; RIFF form header
		{
		<INFO - list>; Supplemental Information
		<sdta - list>; The Sample Binary Data
		<pdta - list>; The Preset, Instrument, and Sample Header data
		}
		)
		*/

	/*<INFO - list> -> LIST(ëINFOí
		{
		<ifil - ck>; Refers to the version of the Sound Font RIFF file
		<isng - ck>; Refers to the target Sound Engine
		<INAM - ck>; Refers to the Sound Font Bank Name
		[<irom - ck>]; Refers to the Sound ROM Name
		[<iver - ck>]; Refers to the Sound ROM Version
		[<ICRD - ck>]; Refers to the Date of Creation of the Bank
		[<IENG - ck>]; Sound Designers and Engineers for the Bank
		[<IPRD - ck>]; Product for which the Bank was intended
		[<ICOP - ck>]; Contains any Copyright message
		[<ICMT - ck>]; Contains any Comments on the Bank
		[<ISFT - ck>]; The SoundFont tools used to create and alter the bank
		}
		)
		*/

	/*<sdta - ck> -> LIST(ësdtaí
		{
		[<smpl - ck>]; The Digital Audio Samples for the upper 16 bits
		}
		{
		[<sm24 - ck>]; The Digital Audio Samples for the lower 8 bits
		}
		)
		*/

	/*<pdta - ck> -> LIST(ëpdtaí
		{
		<phdr - ck>; The Preset Headers
		<pbag - ck>; The Preset Index list
		<pmod - ck>; The Preset Modulator list
		<pgen - ck>; The Preset Generator list
		<inst - ck>; The Instrument Names and Indices
		<ibag - ck>; The Instrument Index list
		<imod - ck>; The Instrument Modulator list
		<igen - ck>; The Instrument Generator list
		<shdr - ck>; The Sample Headers
		}
		)
		*/

	//<iver - rec> -> 
	struct sfVersionTag
	{
		WORD wMajor;
		WORD wMinor;
	};

	//<phdr - rec> -> 
	struct sfPresetHeader
	{
		CHAR achPresetName[20];
		WORD wPreset;
		WORD wBank;
		WORD wPresetBagNdx;
		DWORD dwLibrary;
		DWORD dwGenre;
		DWORD dwMorphology;
	};

	//<pbag - rec> -> 
	struct sfPresetBag
	{
		WORD wGenNdx;
		WORD wModNdx;
	};

	//<pmod - rec> -> 
	struct sfModList  {
		SFModulator sfModSrcOper;
		SFGenerator sfModDestOper;
		SHORT modAmount;
		SFModulator sfModAmtSrcOper;
		SFTransform sfModTransOper;
	};

	//<pgen - rec> -> 
	struct sfGenList
	{
		SFGenerator sfGenOper;
		genAmountType genAmount;
	};

	//<inst - rec> -> 
	struct sfInst
	{
		CHAR achInstName[20];
		WORD wInstBagNdx;
	};

	//<ibag - rec> -> 
	struct sfInstBag
	{
		WORD wInstGenNdx;
		WORD wInstModNdx;
	};

	//<imod - rec> -> 
	struct sfInstModList
	{
		SFModulator sfModSrcOper;
		SFGenerator sfModDestOper;
		SHORT modAmount;
		SFModulator sfModAmtSrcOper;
		SFTransform sfModTransOper;
	};

	//<igen - rec> -> 
	struct sfInstGenList
	{
		SFGenerator sfGenOper;
		genAmountType genAmount;
	};

	//<shdr - rec> -> 
	struct sfSample
	{
		CHAR achSampleName[20];
		DWORD dwStart;
		DWORD dwEnd;
		DWORD dwStartloop;
		DWORD dwEndloop;
		DWORD dwSampleRate;
		BYTE byOriginalKey;
		CHAR chCorrection;
		WORD wSampleLink;
		SFSampleLink sfSampleType;
	};

	//typedef 
	struct rangesType
	{
		BYTE byLo;
		BYTE byHi;
	};

	//typedef 
	union genAmountType
	{
		rangesType ranges;
		SHORT shAmount;
		WORD wAmount;
	};

	//typedef
	enum SFSampleLink 
	{
		monoSample = 1,
		rightSample = 2,
		leftSample = 4,
		linkedSample = 8,
		RomMonoSample = 0x8001,
		RomRightSample = 0x8002,
		RomLeftSample = 0x8004,
		RomLinkedSample = 0x8008
	};

	enum SFGenerator
	{
		startAddrsOffset = 0,	// The offset, in sample data points, beyond the Start sample header parameter to the first sample data point to be played for this instrument.For example, if Start were 7 and startAddrOffset were 2, the first sample data point played would be sample data point 9.
		endAddrsOffset,			// The offset, in sample sample data points, beyond the End sample header parameter to the last sample data point to be played for this instrument.For example, if End were 17 and endAddrOffset were - 2, the last sample data point played would be sample data point 15.
		startloopAddrsOffset,	// The offset, in sample data points, beyond the Startloop sample header parameter to the first sample data point to be repeated in the loop for this instrument.For example, if Startloop were 10 and startloopAddrsOffset were - 1, the first repeated loop sample data point would be sample data point 9.
		endloopAddrsOffset,		// The offset, in sample data points, beyond the Endloop sample header parameter to the sample data point considered equivalent to the Startloop sample data point for the loop for this instrument.For example, if Endloop were 15 and endloopAddrsOffset were 2, sample data point 17 would be considered equivalent to the Startloop sample data point, and hence sample data point 16 would effectively precede Startloop during looping.
		startAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Start sample header parameter and the first sample data point to be played in this instrument.This parameter is added to the startAddrsOffset parameter.For example, if Start were 5, startAddrsOffset were 3 and startAddrsCoarseOffset were 2, the first sample data point played would be sample data point 65544.
		modLfoToPitch,			// This is the degree, in cents, to which a full scale excursion of the Modulation LFO will influence pitch.A positive value indicates a positive LFO excursion increases pitch; a negative value indicates a positive excursion decreases pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will first rise 1 semitone, then fall one semitone.
		vibLfoToPitch,			// This is the degree, in cents, to which a full scale excursion of the Vibrato LFO will influence pitch.A positive value indicates a positive LFO excursion increases pitch; a negative value indicates a positive excursion decreases pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will first rise 1 semitone, then fall one semitone.
		modEnvToPitch,			// This is the degree, in cents, to which a full scale excursion of the Modulation Envelope will influence pitch.A positive value indicates an increase in pitch; a negative value indicates a decrease in pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will rise 1 semitone at the envelope peak.
		initialFilterFc,		// This is the cutoff and resonant frequency of the lowpass filter in absolute cent units.The lowpass filter is defined as a second order resonant pole pair whose pole frequency in Hz is defined by the Initial Filter Cutoff parameter.When the cutoff frequency exceeds 20kHz and the Q(resonance) of the filter is zero, the filter does not affect the signal.
		initialFilterQ,			// This is the height above DC gain in centibels which the filter resonance exhibits at the cutoff frequency.A value of zero or less indicates the filter is not resonant; the gain at the cutoff frequency(pole angle) may be less than zero when zero is specified.The filter gain at DC is also affected by this parameter such that the gain at DC is reduced by half the specified gain.For example, for a value of 100, the filter gain at DC would be 5 dB below unity gain, and the height of the resonant peak would be 10 dB above the DC gain, or 5 dB above unity gain.Note also that if initialFilterQ is set to zero or less and the cutoff frequency exceeds 20 kHz, then the filter response is flat and unity gain.
		modLfoToFilterFc,		// This is the degree, in cents, to which a full scale excursion of the Modulation LFO will influence filter cutoff frequency.A positive number indicates a positive LFO excursion increases cutoff frequency; a negative number indicates a positive excursion decreases cutoff frequency.Filter cutoff frequency is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 1200 indicates that the cutoff frequency will first rise 1 octave, then fall one octave.
		modEnvToFilterFc,		// This is the degree, in cents, to which a full scale excursion of the Modulation Envelope will influence filter cutoff frequency.A positive number indicates an increase in cutoff frequency; a negative number indicates a decrease in filter cutoff frequency.Filter cutoff frequency is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 1000 indicates that the cutoff frequency will rise one octave at the envelope attack peak.
		endAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the End sample header parameter and the last sample data point to be played in this instrument.This parameter is added to the endAddrsOffset parameter.For example, if End were 65536, startAddrsOffset were - 3 and startAddrsCoarseOffset were - 1, the last sample data point played would be sample data point 32765.
		modLfoToVolume,			// This is the degree, in centibels, to which a full scale excursion of the Modulation LFO will influence volume.A positive number indicates a positive LFO excursion increases volume; a negative number indicates a positive excursion decreases volume.Volume is always modified logarithmically, that is the deviation is in decibels rather than in linear amplitude.For example, a value of 100 indicates that the volume will first rise ten dB, then fall ten dB.
		unused1,				// Unused, reserved.Should be ignored if encountered.
		chorusEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the chorus effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the ìdryî or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the chorus effects processor.
		reverbEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the reverb effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the ìdryî or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the reverb effects processor.
		pan,					// This is the degree, in 0.1% units, to which the ìdryî audio output of the note is positioned to the left or right output.A value of - 50 % or less indicates the signal is sent entirely to the left output and not sent to the right output; a value of + 50 % or more indicates the note is sent entirely to the right and not sent to the left.A value of zero places the signal centered between left and right.For example, a value of - 250 indicates that the signal is sent at 75 % of full level to the left output and 25 % of full level to the right output.
		unused2,				// Unused, reserved.Should be ignored if encountered.
		unused3,				// Unused, reserved.Should be ignored if encountered.
		unused4,				// Unused, reserved.Should be ignored if encountered.
		delayModLFO,			// This is the delay time, in absolute timecents, from key on until the Modulation LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second and a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = 7973.
		freqModLFO,				// This is the frequency, in absolute cents, of the Modulation LFOís triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
		delayVibLFO,			// This is the delay time, in absolute timecents, from key on until the Vibrato LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
		freqVibLFO,				// This is the frequency, in absolute cents, of the Vibrato LFOís triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
		delayModEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Modulation envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
		attackModEnv,			// This is the time, in absolute timecents, from the end of the Modulation Envelope Delay Time until the point at which the Modulation Envelope value reaches its peak.Note that the attack is ìconvexî; the curve is nominally such that when applied to a decibel or semitone parameter, the result is linear in amplitude or Hz respectively.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
		holdModEnv,				// This is the time, in absolute timecents, from the end of the attack phase to the entry into decay phase, during which the envelope value is held at its peak.A value of 0 indicates a 1 second hold time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(32768) conventionally indicates no hold phase.For example, a hold time of 10 msec would be 1200log2(.01) = -7973.
		decayModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during decay phase.For the Modulation Envelope, the decay phase linearly ramps toward the sustain level.If the sustain level were zero, the Modulation Envelope Decay Time would be the time spent in decay phase.A value of 0 indicates a 1 second decay time for a zero - sustain level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a decay time of 10 msec would be 1200log2(.01) = -7973.
		sustainModEnv,			// This is the decrease in level, expressed in 0.1% units, to which the Modulation Envelope value ramps during the decay phase.For the Modulation Envelope, the sustain level is properly expressed in percent of full scale.Because the volume envelope sustain level is expressed as an attenuation from full scale, the sustain level is analogously expressed as a decrease from full scale.A value of 0 indicates the sustain level is full level; this implies a zero duration of decay phase regardless of decay time.A positive value indicates a decay to the corresponding level.Values less than zero are to be interpreted as zero; values above 1000 are to be interpreted as 1000.  For example, a sustain level which corresponds to an absolute value 40 % of peak would be 600.
		releaseModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during release phase.For the Modulation Envelope, the release phase linearly ramps toward zero from the current level.If the current level were full scale, the Modulation Envelope Release Time would be the time spent in release phase until zero value were reached.A value of 0 indicates a 1 second decay time for a release from full level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a release time of 10 msec would be 1200log2(.01) = -7973.
		keynumToModEnvHold,		// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
		keynumToModEnvDecay,	// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
		delayVolEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Volume envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
		attackVolEnv,			// This is the time, in absolute timecents, from the end of the Volume Envelope Delay Time until the point at which the Volume Envelope value reaches its peak.Note that the attack is ìconvexî; the curve is nominally such that when applied to the decibel volume parameter, the result is linear in amplitude.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
		holdVolEnv,				// This is the time, in absolute timecents, from the end of the attack phase to the entry into decay phase, during which the Volume envelope value is held at its peak.A value of 0 indicates a 1 second hold time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates no hold phase.For example, a hold time of 10 msec would be 1200log2(.01) = -7973.
		decayVolEnv,			// This is the time, in absolute timecents, for a 100 % change in the Volume Envelope value during decay phase.For the Volume Envelope, the decay phase linearly ramps toward the sustain level, causing a constant dB change for each time unit.If the sustain level were - 100dB, the Volume Envelope Decay Time would be the time spent in decay phase.A value of 0 indicates a 1 - second decay time for a zero - sustain level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a decay time of 10 msec would be 1200log2(.01) = -7973.
		sustainVolEnv,			// This is the decrease in level, expressed in centibels, to which the Volume Envelope value ramps during the decay phase.For the Volume Envelope, the sustain level is best expressed in centibels of attenuation from full scale.A value of 0 indicates the sustain level is full level; this implies a zero duration of decay phase regardless of decay time.A positive value indicates a decay to the corresponding level.Values less than zero are to be interpreted as zero; conventionally 1000 indicates full attenuation.For example, a sustain level which corresponds to an absolute value 12dB below of peak would be 120.
		releaseVolEnv,			// This is the time, in absolute timecents, for a 100 % change in the Volume Envelope value during release phase.For the Volume Envelope, the release phase linearly ramps toward zero from the current level, causing a constant dB change for each time unit.If the current level were full scale, the Volume Envelope Release Time would be the time spent in release phase until 100dB attenuation were reached.A value of 0 indicates a 1 - second decay time for a release from full level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a release time of 10 msec would be 1200log2(.01) = -7973.
		keynumToVolEnvHold,		// This is the degree, in timecents per KeyNumber units, to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Volume Envelope Hold Time were - 7973 = 10 msec and the Key Number to Vol Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
		keynumToVolEnvDecay,	// This is the degree, in timecents per KeyNumber units, to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Volume Envelope Hold Time were - 7973 = 10 msec and the Key Number to Vol Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
		instrument,				// This is the index into the INST sub - chunk providing the instrument to be used for the current preset zone.A value of zero indicates the first instrument in the list.The value should never exceed two less than the size of the instrument list.The instrument enumerator is the terminal generator for PGEN zones.As such, it should only appear in the PGEN sub - chunk, and it must appear as the last generator enumerator in all but the global preset zone.
		reserved1,				// Unused, reserved.Should be ignored if encountered.
		keyRange,				// This is the minimum and maximum MIDI key number values for which this preset zone or instrument zone is active.The LS byte indicates the highest and the MS byte the lowest valid key.The keyRange enumerator is optional, but when it does appear, it must be the first generator in the zone generator list.
		velRange,				// This is the minimum and maximum MIDI velocity values for which this preset zone or instrument zone is active.The LS byte indicates the highest and the MS byte the lowest valid velocity.The velRange enumerator is optional, but when it does appear, it must be preceded only by keyRange in the zone generator list.
		startloopAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Startloop sample header parameter and the first sample data point to be repeated in this instrumentís loop.This parameter is added to the startloopAddrsOffset parameter.For example, if Startloop were 5, startloopAddrsOffset were 3 and startAddrsCoarseOffset were 2, the first sample data point in the loop would be sample data point 65544.
		keynum,					// This enumerator forces the MIDI key number to effectively be interpreted as the value given.This generator can only appear at the instrument level.Valid values are from 0 to 127.
		velocity,				// This enumerator forces the MIDI velocity to effectively be interpreted as the value given.This generator can only appear at the instrument level.Valid values are from 0 to 127.
		initialAttenuation,		// This is the attenuation, in centibels, by which a note is attenuated below full scale.A value of zero indicates no attenuation; the note will be played at full scale.For example, a value of 60 indicates the note will be played at 6 dB below full scale for the note.
		reserved2,				// Unused, reserved.Should be ignored if encountered.
		endloopAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Endloop sample header parameter to the sample data point considered equivalent to the Startloop sample data point for the loop for this instrument.This parameter is added to the endloopAddrsOffset parameter.For example, if Endloop were 5, endloopAddrsOffset were 3 and endAddrsCoarseOffset were 2, sample data point 65544 would be considered equivalent to the Startloop sample data point, and hence sample data point 65543 would effectively precede Startloop during looping.
		coarseTune,				// This is a pitch offset, in semitones, which should be applied to the note.A positive value indicates the sound is reproduced at a higher pitch; a negative value indicates a lower pitch.For example, a Coarse Tune value of - 4 would cause the sound to be reproduced four semitones flat.
		fineTune,				// This is a pitch offset, in cents, which should be applied to the note.It is additive with coarseTune.A positive value indicates the sound is reproduced at a higher pitch; a negative value indicates a lower pitch.For example, a Fine Tuning value of 5 would cause the sound to be reproduced five cents flat.
		sampleID,				// This is the index into the SHDR sub - chunk providing the sample to be used for the current instrument zone.A value of zero indicates the first sample in the list.The value should never exceed two less than the size of the sample list.The sampleID enumerator is the terminal generator for IGEN zones.As such, it should only appear in the IGEN sub - chunk, and it must appear as the last generator enumerator in all but the global zone.
		sampleModes,			// This enumerator indicates a value which gives a variety of Boolean flags describing the sample for the current instrument zone.The sampleModes should only appear in the IGEN sub - chunk, and should not appear in the global zone.The two LS bits of the value indicate the type of loop in the sample : 0 indicates a sound reproduced with no loop, 1 indicates a sound which loops continuously, 2 is unused but should be interpreted as indicating no loop, and 3 indicates a sound which loops for the duration of key depression then proceeds to play the remainder of the sample.
		reserved3,				// Unused, reserved.Should be ignored if encountered.
		scaleTuning,			// This parameter represents the degree to which MIDI key number influences pitch.A value of zero indicates that MIDI key number has no effect on pitch; a value of 100 represents the usual tempered semitone scale.
		exclusiveClass,			// This parameter provides the capability for a key depression in a given instrument to terminate the playback of other instruments.This is particularly useful for percussive instruments such as a hi - hat cymbal.An exclusive class value of zero indicates no exclusive class; no special action is taken.Any other value indicates that when this note is initiated, any other sounding note with the same exclusive class value should be rapidly terminated.The exclusive class generator can only appear at the instrument level.The scope of the exclusive class is the entire preset.In other words, any other instrument zone within the same preset holding a corresponding exclusive class will be terminated.
		overridingRootKey,		// This parameter represents the MIDI key number at which the sample is to be played back at its original sample rate.If not present, or if present with a value of - 1, then the sample header parameter Original Key is used in its place.If it is present in the range 0 - 127, then the indicated key number will cause the sample to be played back at its sample header Sample Rate.For example, if the sample were a recording of a piano middle C(Original Key = 60) at a sample rate of 22.050 kHz, and Root Key were set to 69, then playing MIDI key number 69 (A above middle C) would cause a piano note of pitch middle C to be heard.
		unused5,				// Unused, reserved.Should be ignored if encountered.
		endOper,				// Unused, reserved.Should be ignored if encountered.Unique name provides value to end of defined list
	};

	struct SFModulator
	{
		BYTE Type : 6,	// = A 6 bit value specifying the continuity of the controller  
			 P : 1,		// = Polarity  0= min=0, max=1; 1= min=-1, max=1
			 D : 1,		// = Direction 0=min-max, 1=max->min 
			 CC : 1,	// = MIDI Continuous Controller Flag  
			 Index : 7;	// = A 7 bit value specifying the controller source
	};

	enum SFSource
	{
		NoController = 0,
		NoteOnVelocity = 2,
		NoteOnKeyNumber = 3,
		PolyPressure = 10,
		ChannelPressure = 13,
		PitchWheel = 14,
		PitchWheelSensitivity = 16,
		Link = 127
	};

	enum SFSourceType
	{
		Linear = 0,
		Concave = 1,
		Convex = 2,
		Switch = 3
	};

	enum SFTransform
	{
		Linear = 0,
		AbsoluteValue = 2
	};
	
	
	// - - - “ËÔ˚, Á‡ËÏÒÚ‚Ó‚‡ÌÌ˚Â Ë ÔÂÂ‡·ÓÚ‡ÌÌ˚Â ËÁ fluid
	typedef struct _SFInst
	{				/* Instrument structure */
	  char name[21];		/* Name of instrument */
	  fluid_list_t *zone;			/* list of instrument zones */
	}
	SFInst;

	typedef struct _SFPreset
	{				/* Preset structure */
	  char name[21];		/* preset name */
	  unsigned short prenum;		/* preset number */
	  unsigned short bank;			/* bank number */
	  unsigned int libr;			/* Not used (preserved) */
	  unsigned int genre;		/* Not used (preserved) */
	  unsigned int morph;		/* Not used (preserved) */
	  fluid_list_t *zone;			/* list of preset zones */
	}
	SFPreset;

	/* NOTE: sffd is also used to determine if sound font is new (NULL) */
	typedef struct _SFData
	{				/* Sound font data structure */
	  SFVersion version;		/* sound font version */
	  SFVersion romver;		/* ROM version */
	  unsigned int samplepos;		/* position within sffd of the sample chunk */
	  unsigned int samplesize;		/* length within sffd of the sample chunk */
	  char *fname;			/* file name */
	  FILE *sffd;			/* loaded sfont file descriptor */
	  fluid_list_t *info;		     /* linked list of info strings (1st byte is ID) */
	  fluid_list_t *preset;		/* linked list of preset info */
	  fluid_list_t *inst;			/* linked list of instrument info */
	  fluid_list_t *sample;		/* linked list of sample info */
	}
	SFData;

	/* sf file chunk IDs */
	enum
	{ UNKN_ID, RIFF_ID, LIST_ID, SFBK_ID,
	  INFO_ID, SDTA_ID, PDTA_ID,	/* info/sample/preset */

	  IFIL_ID, ISNG_ID, INAM_ID, IROM_ID, /* info ids (1st byte of info strings) */
	  IVER_ID, ICRD_ID, IENG_ID, IPRD_ID,	/* more info ids */
	  ICOP_ID, ICMT_ID, ISFT_ID,	/* and yet more info ids */

	  SNAM_ID, SMPL_ID,		/* sample ids */
	  PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,	/* preset ids */
	  IHDR_ID, IBAG_ID, IMOD_ID, IGEN_ID,	/* instrument ids */
	  SHDR_ID			/* sample info */
	};

	
	/*-----------------------------------sffile.h----------------------------*/
	/*
	   File structures and routines (used to be in sffile.h)
	*/

	#define CHNKIDSTR(id)           &idlist[(id - 1) * 4]

	/* sfont file chunk sizes */
	#define SFPHDRSIZE	38
	#define SFBAGSIZE	4
	#define SFMODSIZE	10
	#define SFGENSIZE	4
	#define SFIHDRSIZE	22
	#define SFSHDRSIZE	46

	/* sfont file data structures */
	typedef struct _SFChunk
	{				/* RIFF file chunk structure */
	  unsigned int id;			/* chunk id */
	  unsigned int size;			/* size of the following chunk */
	}
	SFChunk;

	typedef struct _SFPhdr
	{
	  unsigned char name[20];		/* preset name */
	  unsigned short preset;		/* preset number */
	  unsigned short bank;			/* bank number */
	  unsigned short pbagndx;		/* index into preset bag */
	  unsigned int library;		/* just for preserving them */
	  unsigned int genre;		/* Not used */
	  unsigned int morphology;		/* Not used */
	}
	SFPhdr;

	typedef struct _SFBag
	{
	  unsigned short genndx;		/* index into generator list */
	  unsigned short modndx;		/* index into modulator list */
	}
	SFBag;

	typedef struct _SFIhdr
	{
	  char name[20];		/* Name of instrument */
	  unsigned short ibagndx;		/* Instrument bag index */
	}
	SFIhdr;

	typedef struct _SFShdr
	{				/* Sample header loading struct */
	  char name[20];		/* Sample name */
	  unsigned int start;		/* Offset to start of sample */
	  unsigned int end;			/* Offset to end of sample */
	  unsigned int loopstart;		/* Offset to start of loop */
	  unsigned int loopend;		/* Offset to end of loop */
	  unsigned int samplerate;		/* Sample rate recorded at */
	  unsigned char origpitch;		/* root midi key number */
	  signed char pitchadj;		/* pitch correction in cents */
	  unsigned short samplelink;		/* Not used */
	  unsigned short sampletype;		/* 1 mono,2 right,4 left,linked 8,0x8000=ROM */
	}
	SFShdr;

	/* data */
	extern char idlist[];

	/* functions */
	SFData *sfload_file (const char * fname);

	// - - - ≈Ÿ®
	/*

	  Public interface

	 */

	fluid_sfloader_t* new_fluid_defsfloader(void);
	int delete_fluid_defsfloader(fluid_sfloader_t* loader);
	fluid_sfont_t* fluid_defsfloader_load(fluid_sfloader_t* loader, const char* filename);


	int fluid_defsfont_sfont_delete(fluid_sfont_t* sfont);
	char* fluid_defsfont_sfont_get_name(fluid_sfont_t* sfont);
	fluid_preset_t* fluid_defsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);
	void fluid_defsfont_sfont_iteration_start(fluid_sfont_t* sfont);
	int fluid_defsfont_sfont_iteration_next(fluid_sfont_t* sfont, fluid_preset_t* preset);


	int fluid_defpreset_preset_delete(fluid_preset_t* preset);
	char* fluid_defpreset_preset_get_name(fluid_preset_t* preset);
	int fluid_defpreset_preset_get_banknum(fluid_preset_t* preset);
	int fluid_defpreset_preset_get_num(fluid_preset_t* preset);
	int fluid_defpreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);


	/*
	 * fluid_defsfont_t
	 */
	struct _fluid_defsfont_t
	{
	  char* filename;           /* the filename of this soundfont */
	  unsigned int samplepos;   /* the position in the file at which the sample data starts */
	  unsigned int samplesize;  /* the size of the sample data */
	  short* sampledata;        /* the sample data, loaded in ram */
	  fluid_list_t* sample;      /* the samples in this soundfont */
	  fluid_defpreset_t* preset; /* the presets of this soundfont */

	  fluid_preset_t iter_preset;        /* preset interface used in the iteration */
	  fluid_defpreset_t* iter_cur;       /* the current preset in the iteration */
	};


	fluid_defsfont_t* new_fluid_defsfont(void);
	int delete_fluid_defsfont(fluid_defsfont_t* sfont);
	int fluid_defsfont_load(fluid_defsfont_t* sfont, const char* file);
	char* fluid_defsfont_get_name(fluid_defsfont_t* sfont);
	fluid_defpreset_t* fluid_defsfont_get_preset(fluid_defsfont_t* sfont, unsigned int bank, unsigned int prenum);
	void fluid_defsfont_iteration_start(fluid_defsfont_t* sfont);
	int fluid_defsfont_iteration_next(fluid_defsfont_t* sfont, fluid_preset_t* preset);
	int fluid_defsfont_load_sampledata(fluid_defsfont_t* sfont);
	int fluid_defsfont_add_sample(fluid_defsfont_t* sfont, fluid_sample_t* sample);
	int fluid_defsfont_add_preset(fluid_defsfont_t* sfont, fluid_defpreset_t* preset);


	/*
	 * fluid_preset_t
	 */
	struct _fluid_defpreset_t
	{
	  fluid_defpreset_t* next;
	  fluid_defsfont_t* sfont;                  /* the soundfont this preset belongs to */
	  char name[21];                        /* the name of the preset */
	  unsigned int bank;                    /* the bank number */
	  unsigned int num;                     /* the preset number */
	  fluid_preset_zone_t* global_zone;        /* the global zone of the preset */
	  fluid_preset_zone_t* zone;               /* the chained list of preset zones */
	};

	fluid_defpreset_t* new_fluid_defpreset(fluid_defsfont_t* sfont);
	int delete_fluid_defpreset(fluid_defpreset_t* preset);
	fluid_defpreset_t* fluid_defpreset_next(fluid_defpreset_t* preset);
	int fluid_defpreset_import_sfont(fluid_defpreset_t* preset, SFPreset* sfpreset, fluid_defsfont_t* sfont);
	int fluid_defpreset_set_global_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone);
	int fluid_defpreset_add_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone);
	fluid_preset_zone_t* fluid_defpreset_get_zone(fluid_defpreset_t* preset);
	fluid_preset_zone_t* fluid_defpreset_get_global_zone(fluid_defpreset_t* preset);
	int fluid_defpreset_get_banknum(fluid_defpreset_t* preset);
	int fluid_defpreset_get_num(fluid_defpreset_t* preset);
	char* fluid_defpreset_get_name(fluid_defpreset_t* preset);
	int fluid_defpreset_noteon(fluid_defpreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

	/*
	 * fluid_preset_zone
	 */
	struct _fluid_preset_zone_t
	{
	  fluid_preset_zone_t* next;
	  char* name;
	  fluid_inst_t* inst;
	  int keylo;
	  int keyhi;
	  int vello;
	  int velhi;
	  fluid_gen_t gen[GEN_LAST];
	  fluid_mod_t * mod; /* List of modulators */
	};

	fluid_preset_zone_t* new_fluid_preset_zone(char* name);
	int delete_fluid_preset_zone(fluid_preset_zone_t* zone);
	fluid_preset_zone_t* fluid_preset_zone_next(fluid_preset_zone_t* preset);
	int fluid_preset_zone_import_sfont(fluid_preset_zone_t* zone, SFZone* sfzone, fluid_defsfont_t* sfont);
	int fluid_preset_zone_inside_range(fluid_preset_zone_t* zone, int key, int vel);
	fluid_inst_t* fluid_preset_zone_get_inst(fluid_preset_zone_t* zone);

	/*
	 * fluid_inst_t
	 */
	struct _fluid_inst_t
	{
	  char name[21];
	  fluid_inst_zone_t* global_zone;
	  fluid_inst_zone_t* zone;
	};

	fluid_inst_t* new_fluid_inst(void);
	int delete_fluid_inst(fluid_inst_t* inst);
	int fluid_inst_import_sfont(fluid_inst_t* inst, SFInst *sfinst, fluid_defsfont_t* sfont);
	int fluid_inst_set_global_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone);
	int fluid_inst_add_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone);
	fluid_inst_zone_t* fluid_inst_get_zone(fluid_inst_t* inst);
	fluid_inst_zone_t* fluid_inst_get_global_zone(fluid_inst_t* inst);

	/*
	 * fluid_inst_zone_t
	 */
	struct _fluid_inst_zone_t
	{
	  fluid_inst_zone_t* next;
	  char* name;
	  fluid_sample_t* sample;
	  int keylo;
	  int keyhi;
	  int vello;
	  int velhi;
	  fluid_gen_t gen[GEN_LAST];
	  fluid_mod_t * mod; /* List of modulators */
	};

	fluid_inst_zone_t* new_fluid_inst_zone(char* name);
	int delete_fluid_inst_zone(fluid_inst_zone_t* zone);
	fluid_inst_zone_t* fluid_inst_zone_next(fluid_inst_zone_t* zone);
	int fluid_inst_zone_import_sfont(fluid_inst_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont);
	int fluid_inst_zone_inside_range(fluid_inst_zone_t* zone, int key, int vel);
	fluid_sample_t* fluid_inst_zone_get_sample(fluid_inst_zone_t* zone);



	fluid_sample_t* new_fluid_sample(void);
	int delete_fluid_sample(fluid_sample_t* sample);
	int fluid_sample_import_sfont(fluid_sample_t* sample, SFSample* sfsample, fluid_defsfont_t* sfont);
	int fluid_sample_in_rom(fluid_sample_t* sample);

	/*
	* fluid_defpreset_noteon
	*/
	int
		fluid_defpreset_noteon(fluid_defpreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel)
	{
		fluid_preset_zone_t *preset_zone, *global_preset_zone;
		fluid_inst_t* inst;
		fluid_inst_zone_t *inst_zone, *global_inst_zone, *z;
		fluid_sample_t* sample;
		fluid_voice_t* voice;
		fluid_mod_t * mod;
		fluid_mod_t * mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */
		int mod_list_count;
		int i;

		global_preset_zone = fluid_defpreset_get_global_zone(preset);

		/* run thru all the zones of this preset */
		preset_zone = fluid_defpreset_get_zone(preset);
		while (preset_zone != NULL) {

			/* check if the note falls into the key and velocity range of this
			preset */
			if (fluid_preset_zone_inside_range(preset_zone, key, vel)) {

				inst = fluid_preset_zone_get_inst(preset_zone);
				global_inst_zone = fluid_inst_get_global_zone(inst);

				/* run thru all the zones of this instrument */
				inst_zone = fluid_inst_get_zone(inst);
				while (inst_zone != NULL) {

					/* make sure this instrument zone has a valid sample */
					sample = fluid_inst_zone_get_sample(inst_zone);
					if (fluid_sample_in_rom(sample) || (sample == NULL)) {
						inst_zone = fluid_inst_zone_next(inst_zone);
						continue;
					}

					/* check if the note falls into the key and velocity range of this
					instrument */

					if (fluid_inst_zone_inside_range(inst_zone, key, vel) && (sample != NULL)) {

						/* this is a good zone. allocate a new synthesis process and
						initialize it */

						voice = fluid_synth_alloc_voice(synth, sample, chan, key, vel);
						if (voice == NULL) {
							return FLUID_FAILED;
						}


						z = inst_zone;

						/* Instrument level, generators */

						for (i = 0; i < GEN_LAST; i++) {

							/* SF 2.01 section 9.4 'bullet' 4:
							*
							* A generator in a local instrument zone supersedes a
							* global instrument zone generator.  Both cases supersede
							* the default generator -> voice_gen_set */

							if (inst_zone->gen[i].flags){
								fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

							}
							else if ((global_inst_zone != NULL) && (global_inst_zone->gen[i].flags)) {
								fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

							}
							else {
								/* The generator has not been defined in this instrument.
								* Do nothing, leave it at the default.
								*/
							}

						} /* for all generators */

						/* global instrument zone, modulators: Put them all into a
						* list. */

						mod_list_count = 0;

						if (global_inst_zone){
							mod = global_inst_zone->mod;
							while (mod){
								mod_list[mod_list_count++] = mod;
								mod = mod->next;
							}
						}

						/* local instrument zone, modulators.
						* Replace modulators with the same definition in the list:
						* SF 2.01 page 69, 'bullet' 8
						*/
						mod = inst_zone->mod;

						while (mod){

							/* 'Identical' modulators will be deleted by setting their
							*  list entry to NULL.  The list length is known, NULL
							*  entries will be ignored later.  SF2.01 section 9.5.1
							*  page 69, 'bullet' 3 defines 'identical'.  */

							for (i = 0; i < mod_list_count; i++){
								if (mod_list[i] && fluid_mod_test_identity(mod, mod_list[i])){
									mod_list[i] = NULL;
								}
							}

							/* Finally add the new modulator to to the list. */
							mod_list[mod_list_count++] = mod;
							mod = mod->next;
						}

						/* Add instrument modulators (global / local) to the voice. */
						for (i = 0; i < mod_list_count; i++){

							mod = mod_list[i];

							if (mod != NULL){ /* disabled modulators CANNOT be skipped. */

								/* Instrument modulators -supersede- existing (default)
								* modulators.  SF 2.01 page 69, 'bullet' 6 */
								fluid_voice_add_mod(voice, mod, FLUID_VOICE_OVERWRITE);
							}
						}

						/* Preset level, generators */

						for (i = 0; i < GEN_LAST; i++) {

							/* SF 2.01 section 8.5 page 58: If some generators are
							* encountered at preset level, they should be ignored */
							if ((i != GEN_STARTADDROFS)
								&& (i != GEN_ENDADDROFS)
								&& (i != GEN_STARTLOOPADDROFS)
								&& (i != GEN_ENDLOOPADDROFS)
								&& (i != GEN_STARTADDRCOARSEOFS)
								&& (i != GEN_ENDADDRCOARSEOFS)
								&& (i != GEN_STARTLOOPADDRCOARSEOFS)
								&& (i != GEN_KEYNUM)
								&& (i != GEN_VELOCITY)
								&& (i != GEN_ENDLOOPADDRCOARSEOFS)
								&& (i != GEN_SAMPLEMODE)
								&& (i != GEN_EXCLUSIVECLASS)
								&& (i != GEN_OVERRIDEROOTKEY)) {

								/* SF 2.01 section 9.4 'bullet' 9: A generator in a
								* local preset zone supersedes a global preset zone
								* generator.  The effect is -added- to the destination
								* summing node -> voice_gen_incr */

								if (preset_zone->gen[i].flags) {
									fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
								}
								else if ((global_preset_zone != NULL) && global_preset_zone->gen[i].flags) {
									fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
								}
								else {
									/* The generator has not been defined in this preset
									* Do nothing, leave it unchanged.
									*/
								}
							} /* if available at preset level */
						} /* for all generators */


						/* Global preset zone, modulators: put them all into a
						* list. */
						mod_list_count = 0;
						if (global_preset_zone){
							mod = global_preset_zone->mod;
							while (mod){
								mod_list[mod_list_count++] = mod;
								mod = mod->next;
							}
						}

						/* Process the modulators of the local preset zone.  Kick
						* out all identical modulators from the global preset zone
						* (SF 2.01 page 69, second-last bullet) */

						mod = preset_zone->mod;
						while (mod){
							for (i = 0; i < mod_list_count; i++){
								if (mod_list[i] && fluid_mod_test_identity(mod, mod_list[i])){
									mod_list[i] = NULL;
								}
							}

							/* Finally add the new modulator to the list. */
							mod_list[mod_list_count++] = mod;
							mod = mod->next;
						}

						/* Add preset modulators (global / local) to the voice. */
						for (i = 0; i < mod_list_count; i++){
							mod = mod_list[i];
							if ((mod != NULL) && (mod->amount != 0)) { /* disabled modulators can be skipped. */

								/* Preset modulators -add- to existing instrument /
								* default modulators.  SF2.01 page 70 first bullet on
								* page */
								fluid_voice_add_mod(voice, mod, FLUID_VOICE_ADD);
							}
						}

						/* add the synthesis process to the synthesis loop. */
						fluid_synth_start_voice(synth, voice);

						/* Store the ID of the first voice that was created by this noteon event.
						* Exclusive class may only terminate older voices.
						* That avoids killing voices, which have just been created.
						* (a noteon event can create several voice processes with the same exclusive
						* class - for example when using stereo samples)
						*/
					}

					inst_zone = fluid_inst_zone_next(inst_zone);
				}
			}
			preset_zone = fluid_preset_zone_next(preset_zone);
		}

		return FLUID_OK;
	}
}