#pragma once
#include "pch.h"
#include "math.h"
#include "ppltasks.h"
#include "agents.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace concurrency;

#define SOURCE_VOICES_MAX		16		// ������������ ���������� �������
#define DISCRETIZATION_FREQ		44100	// ������� �������������
#define MAX_TEXT_LENGTH			255		// ������������ ����� ����� ������ ������ ~ 64 ����� 4/4. ������ ������ - �������� �� ����� ������

/**
 * ������������ ���, ������������ ������� ������ ��� ���������� ������������ ���������.
 */
namespace SketchMusic
{
	const int TICK_IN_BEAT = 32;	// ���������� ����� � ����
	const int BPM = 120;
	const int MAX_COMMAND_COUNT = 20; // ������������ ����� ������ � �������
	const int MIDIKEYTOA4 = 69;

	ref class Text;
	ref class TextIterator;
	ref class Cursor;
	ref class PartDefinition;
	ref class Idea;
	ref class CompositionHeader;
	ref class CompositionData;
	//ref class CompositionLibrary;
	ref class Composition;
	ref class PositionedIdea;
	
	public enum class NoteType
	{
		A	= 0,
		As	= 1,
		B	= 2,
		C	= 3,
		Cs	= 4,
		D	= 5,
		Ds	= 6,
		E	= 7,
		F	= 8,
		Fs	= 9,
		G	= 10,
		Gs	= 11,
	};

	Platform::String^ noteTypeToString(NoteType note);

	public enum class IdeaCategoryEnum
		// ���������, � ������� ��������� ����. ����� �������������� 
		// ��� ������ ����������� ��������� ���� � ��� ����������. ����� ��� ���-������
		// TODO : ��������� ��������� (?) : functional (������, ���������)
		// transitions (�������� - ����������� ������ �������), loops 
	{
		none = 0,		// �� ��������� ����� �������, ��� ���� �� ����������� ������� ���������.
			// ���� ���, ������������ ����� ���������� "���������������" ���� ...
		name = 1,		// ���� ���������� �� ���� ������ ��������
		instrument = 2,	// ���� ���������� ������������ ���������� (��� ������� - ������ ���������������)
		tempo = 6,		// ����� ���� � ������. ����� ������������ ��� ���� ��� �������� ����������,
						// ����� ������ �� ������ � ��� ������.

		rhythm = 10,			// ����� ��������� ����
		melody = 11,			// ������������ ������������������ + ����
		motive = 12,			// ��� �������, ������ ������ (16 ��� ������ 64)
		chord = 13,				// ���� ������
		melodicSequence = 15,	// ������������ ������������������ - ����� ��� ��� �������� � �����
		chordProgression = 16,	// ������������� ������������������ - ����� �������� ��� �������� � �����?

		shape = 20,		// ����� ����������. ����� ��������� ���� ����������� ������ ����, �������� ������,
		// ��� ��� ����� ������� � �������������� ���� � �������� ������ - � ������� ����������� "�������� �����"?
		// "������� �����" ����� ���������, ��� ���������� ���� ����� � ������������� ������.
		// ������� "�������� �� ����� ������" ����� ���� ����� ������ �������� �����
		// TODO : ����� ������ ������ ����� ����� / �������� - ��� ��������� ���� ��������? 
		// �������� ������� ���� ��� � ��������� "������������� �������" - ����� ����� ����� ����������� ��� ������� � ���� ��������/������
		// TODO : � ������ ��������� ����� ������ ����� ���������� ������������ �������� ��� ��� �����-������ ����
		// TODO : ������������ ������?

		dynamic = 30,	// �������� ��������/�������� (���������). ��� ���� � �������������� ������,
		// ������ � ����� ����� ����, ��� �������������

		noise = 40,		// ������� �������
		effects = 50,	// ������� � ���� ��������� ����� - ������ ��� � ����

		lyrics = 90,	// ����� ����� - ��� �� ������� � ��������
		combo = 100		// ���� ���������� ����� ���� ����������
	};

	// ��������� ��������. ������ ����� ������� ����, ����� ����� ������ ����� ���� ���������� ����� ��������� ���������.
	public enum class DynamicCategory
	{
		quite		= 0,	// |--
		regular		= 1,	// ||
		unregular	= 2, 	// |/|
		fast		= 3,	// ||||
		hard		= 4,	// !
		harder		= 5,	// !||
		solo		= 6,	// ~
	};

	ref class Sample;
	ref class Instrument;
	ref class InstrumentToTextConverter;

	// ���������� ���������

	// ������ ������������ ����� ��������
	public enum class SymbolType
	{
		unknown	= 0,	// ��� �� �������� ��� ��� �� ����� (���� ������������ � �������� ���������)
		
		NOTE	= 1,	// ������� ����
		RNOTE	= 2,	// ������������� ���� (������������ ��������� "�������")
		GNOTE	= 3,	// ���������� ���� (�� ����� ���������������� ����� � ������� �����)
		END		= 4,	// ����������� �������� ���(-�)
		CHORD	= 5,	// ����� �������� ��� SCALE - ���������� ������ (� ����������, �������������������� ����) / �������� / ��������

		NLINE	= 6,	// ����� ������ - ��� ����������� �����������
		SPACE	= 7,	// "������" - ��� ����������� �����������
		NPART	= 8,	// ����� ������ - ��� ��������� ������ ���� �� ������

		TEMPO	= 9,	// ������� �����
		STRING	= 10,	// ��� ������� ������		
		CLEF	= 11,	// ��� ��������� ��� �� ���������
		ACCENT	= 12,	// ��� "���������" ������� ��������� - ����� ������� ����� �������� �������� �����.
						// �� ����, �� �� ����� ���� ����� ��������� GNote, �� ���� ������� ����� �����, ���� ������ ��������,
						// � ����� ����� ������� ��� �������� "��������" ���������

		SCALE	= 15,	// �����, ��������
		HARMONY = 16,	// ������� ��������
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class IdeaCategoryToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			if (value == nullptr) return nullptr;
			IdeaCategoryEnum type = (IdeaCategoryEnum)(value);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "������";
			case IdeaCategoryEnum::chordProgression: return "������������������ ��������";
			case IdeaCategoryEnum::combo: return "��������������� ����";
			case IdeaCategoryEnum::dynamic: return "��������";
			case IdeaCategoryEnum::instrument: return "����������";
			case IdeaCategoryEnum::lyrics: return L"\uE8E4";
			case IdeaCategoryEnum::melodicSequence: return L"\ue189";
			case IdeaCategoryEnum::melody: return L"\ue189";
			case IdeaCategoryEnum::name: return "��������";
			case IdeaCategoryEnum::rhythm: return "����";
			case IdeaCategoryEnum::shape: return "�����";
			case IdeaCategoryEnum::tempo: return "����";
			case IdeaCategoryEnum::none:
			default: return "����������� ���";
			}
			return "";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;
			int res = std::wcstol(dynamic_cast<String^>(value)->Data(), NULL, 10);
			return res;
		}

		IdeaCategoryToStrConverter() {}
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class VerboseIdeaCategoryToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			if (value == nullptr) return nullptr;
			IdeaCategoryEnum type = (IdeaCategoryEnum)(value);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "������";
			case IdeaCategoryEnum::chordProgression: return "������������������ ��������";
			case IdeaCategoryEnum::combo: return "��������������� ����";
			case IdeaCategoryEnum::dynamic: return "��������";
			case IdeaCategoryEnum::instrument: return "����������";
			case IdeaCategoryEnum::lyrics: return "�����/�����";
			case IdeaCategoryEnum::melodicSequence: return "������������ ������������������";
			case IdeaCategoryEnum::melody: return "�������";
			case IdeaCategoryEnum::name: return "��������";
			case IdeaCategoryEnum::rhythm: return "����";
			case IdeaCategoryEnum::shape: return "�����";
			case IdeaCategoryEnum::tempo: return "����";
			case IdeaCategoryEnum::none:
			default: return "����������� ���";
			}
			return "";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;
			int res = std::wcstol(dynamic_cast<String^>(value)->Data(), NULL, 10);
			return res;
		}

		VerboseIdeaCategoryToStrConverter() {}
	};

	ref class TimeToStrConverter;
	ref class VerbosePosIdeaCategoryToStrConverter;

	public interface class ISymbol
	{
	public:
		SymbolType GetSymType();
		Platform::String^ ToString();	// ������������� � ��������� ���� ��������
		
		bool EQ(ISymbol^ second);
	};

	public interface class INote
	{
	public:
		property int _val;
		property int _velocity;
		property int _voice;
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class PositionedSymbol sealed
	{
	public:
		property SketchMusic::Cursor^ _pos;
		property SketchMusic::ISymbol^ _sym;

		PositionedSymbol() { }
		PositionedSymbol(Cursor^ pos, ISymbol^ sym) { _pos = pos; _sym = sym; }
	};

	partial ref class SNote;
	
	partial ref class SSpace;

	// ������ - ��� ����������, ��� � ��������.
	// ��� ���� ����� ����������� ����������� ��������� ������������� - ���� ����������� ���� ������,
	// �� ��� ����������� �� �������� "����", �� ������������� �� ��������� ���� - ������������� ������ ���� "��� ���������"
	// �������������� ����� ������ ��������. ��� ������� ����� ����������� ��� ��������; ��� 
	public ref class SAccent sealed : public SketchMusic::ISymbol
	{
	public:
		SAccent() {}

		virtual SymbolType GetSymType() { return SymbolType::ACCENT; }
		virtual Platform::String^ ToString() { return L"\uE9A9"; }

		// ������������ ����� ISymbol
		virtual bool EQ(ISymbol ^ second);
	};


	const int TrebleClef	= -5;	// ���������� ����; ������ ������ = ��4
	const int BassClef		= -14;	// ������� ����; ������ ������ = ����3
	//const int AltoClef		= 0;
	//const int TenorClef		= 0;
	//const int BaritoneClef	= 0;

	partial ref class SNewPart;
	partial ref class PartDefinition;

	partial ref class STempo;
	partial ref class SRNote;
	partial ref class SGNote;
	partial ref class SNoteEnd;
	partial ref class SScale;

	public enum class Degree
	{
		I	= 1,
		II	= 2,
		III	= 3,
		IV	= 4,
		V	= 5,
		VI	= 6,
		VII	= 7,
	};

	// ���������� ��������� ������������� ������������ ������
	public enum class DegreeInfo
	{
		Tone = 0,
		Dim = 1,
		Aug = 2,
	};

	public enum class DegreeMod
	{
		Normal = 0,
		Flat = -1,
		Sharp = 1,
	};

	// ������� �������� ����� ������������ �������� ������� �������,
	// �� ������� ����� "�������������" ������������� ����
	public ref class SHarmony sealed : public ISymbol
	{
	public:
		SHarmony(int val) { _val = val; }

		// ������������ ����� ISymbol
		virtual SymbolType GetSymType() { return SymbolType::HARMONY; };
		virtual Platform::String ^ ToString();
		virtual bool EQ(ISymbol ^ second);

		property int _val;
	};

	// ������������� � LinStart � ����� �� ������ ��� ����������� ����������
	// ��� ������� ������, ����� ����� ���� � ������������� �������� �� ������ ������,
	// � ��� ����� ������ ����� ���� ������������ �������������
	// ���� ����� ������ � ������ ������ ������� ��� ����������� �������, 
	// � ���� ������������ ������ ��� ��������������.
	public ref class SNewLine sealed : public ISymbol
	{
	public:
		SNewLine() {}

		virtual SymbolType GetSymType() { return SymbolType::NLINE; }
		virtual Platform::String^ ToString() { return L"\uE751"; }

		// ������������ ����� ISymbol
		virtual bool EQ(ISymbol ^ second);
	};

	/**
	����� ����� ���� ��������� ����� � ����������
	*/
	public ref class SString sealed : public ISymbol
	{
	public:
		SString() {}

		property String^ value;

		virtual SymbolType GetSymType() { return SymbolType::STRING; }
		virtual Platform::String^ ToString() { return value; }

		// ������������ ����� ISymbol
		virtual bool EQ(ISymbol ^ second);
	};

	namespace SerializationTokens
	{
		static Platform::String^ TEXT = "text";		// ����� ����� ���� � ���� ������ ��������� �������
		static Platform::String^ INSTR = "instr";		// �������� �����������
		static Platform::String^ INSTR_FNAME = "file";		// ��� ����� � ������������
		static Platform::String^ INSTR_PRESET = "pset";		// �������� �����������
		static Platform::String^ BEAT = "p1";			// beat
		static Platform::String^ TICK = "p2";			// tick
		static Platform::String^ SYMBOL_TYPE = "t";		// ��� �������
		static Platform::String^ NOTE_VALUE = "v";		// ��������
		static Platform::String^ NOTE_VELOCITY = "y";	// ��������
		static Platform::String^ NOTE_VOICE = "o";		// ����� ������
		static Platform::String^ NOTES_ARRAY = "notes";	// ����������� ������� � ������
		static Platform::String^ CONTROL_TEXT = "ctrl_txt";	// ����������� ������, ����������� ����� ��� ���� "����������������" ������� �������
		static Platform::String^ TEXTS_ARRAY = "texts";	// ����������� ������� � ��������
		static Platform::String^ IDEAS_ARRAY = "ideas";	// ����������� ������� � ������
		static Platform::String^ PROJ_NAME = "name";	// ��� �������
		static Platform::String^ PROJ_DESC = "descr";	// �������� �������
		static Platform::String^ PROJ_BPM = "bpm";		// ������� ���� �������
		static Platform::String^ PART_NAME = "n";		// �������� �����
		static Platform::String^ PART_CAT = "s";		// ��������� �����
		static Platform::String^ PART_DYN = "d";		// �������� �����

		static Platform::String^ SCALE_BASE = "s";		// ������� ���� �����
		static Platform::String^ SCALE_1 = "I";			// ������ ������� �����
		static Platform::String^ SCALE_2 = "II";		// ������ ������� �����
		static Platform::String^ SCALE_3 = "III";		// 
		static Platform::String^ SCALE_4 = "IV";		// 
		static Platform::String^ SCALE_5 = "V";			// 
		static Platform::String^ SCALE_6 = "VI";		// 
		static Platform::String^ SCALE_7 = "VII";		// 

		static Platform::String^ IDEA_HASH = "h";	// ���
		static Platform::String^ IDEA_NAME = "n";	// �������� ����
		static Platform::String^ IDEA_MOD = "m";	// ����� ���������
		static Platform::String^ IDEA_CRE = "t";	// ����� ��������
		static Platform::String^ IDEA_CAT = "c";	// ��������� ����
		static Platform::String^ IDEA_CONTENT = "s";	// ��������������� ����������
		static Platform::String^ IDEA_PARENT = "a";	// ��� ������������ ����
		static Platform::String^ IDEA_DESC = "d";	// ��������
		static Platform::String^ IDEA_RATE = "r";	// �������
		static Platform::String^ IDEA_TAGS = "g";	// ����
		static Platform::String^ PIDEA_POS = "p";	// ��������� ���� � ����� Beat
		static Platform::String^ PIDEA_LEN = "l";	// ����� ���� � ����� Beat
		static Platform::String^ PIDEA_LAY = "i";	// ���� ����

		static const std::map<Degree, Platform::String^> degreeSerializationString =
		{
			{ Degree::I,	SCALE_1 },
			{ Degree::II,	SCALE_2 },
			{ Degree::III,	SCALE_3 },
			{ Degree::IV,	SCALE_4 },
			{ Degree::V,	SCALE_5 },
			{ Degree::VI,	SCALE_6 },
			{ Degree::VII,	SCALE_7 },
		};
	}

	// ������ ��� ����������� ������ ������� SoundFont
	// ������ ��� ���������, ����� ������� �������
	namespace SFReader
	{
		ref class SFInstrument;
		ref class SFInstrumentZone;
		ref class SFPresetZone;
		ref class SFPreset;
		ref class SFData;
		ref class SFSample;
		ref class SoundFont;
		ref class SFGenerator;
		ref class SFModulator;
		ref class SFVersion;
		ref class SFLoader;
		ref class SFChunk;

		public enum class SFGeneratorID
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
			chorusEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the chorus effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the �dry� or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the chorus effects processor.
			reverbEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the reverb effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the �dry� or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the reverb effects processor.
			pan,					// This is the degree, in 0.1% units, to which the �dry� audio output of the note is positioned to the left or right output.A value of - 50 % or less indicates the signal is sent entirely to the left output and not sent to the right output; a value of + 50 % or more indicates the note is sent entirely to the right and not sent to the left.A value of zero places the signal centered between left and right.For example, a value of - 250 indicates that the signal is sent at 75 % of full level to the left output and 25 % of full level to the right output.
			unused2,				// Unused, reserved.Should be ignored if encountered.
			unused3,				// Unused, reserved.Should be ignored if encountered.
			unused4,				// Unused, reserved.Should be ignored if encountered.
			delayModLFO,			// This is the delay time, in absolute timecents, from key on until the Modulation LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second and a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = 7973.
			freqModLFO,				// This is the frequency, in absolute cents, of the Modulation LFO�s triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
			delayVibLFO,			// This is the delay time, in absolute timecents, from key on until the Vibrato LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			freqVibLFO,				// This is the frequency, in absolute cents, of the Vibrato LFO�s triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
			delayModEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Modulation envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			attackModEnv,			// This is the time, in absolute timecents, from the end of the Modulation Envelope Delay Time until the point at which the Modulation Envelope value reaches its peak.Note that the attack is �convex�; the curve is nominally such that when applied to a decibel or semitone parameter, the result is linear in amplitude or Hz respectively.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
			holdModEnv,				// This is the time, in absolute timecents, from the end of the attack phase to the entry into decay phase, during which the envelope value is held at its peak.A value of 0 indicates a 1 second hold time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(32768) conventionally indicates no hold phase.For example, a hold time of 10 msec would be 1200log2(.01) = -7973.
			decayModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during decay phase.For the Modulation Envelope, the decay phase linearly ramps toward the sustain level.If the sustain level were zero, the Modulation Envelope Decay Time would be the time spent in decay phase.A value of 0 indicates a 1 second decay time for a zero - sustain level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a decay time of 10 msec would be 1200log2(.01) = -7973.
			sustainModEnv,			// This is the decrease in level, expressed in 0.1% units, to which the Modulation Envelope value ramps during the decay phase.For the Modulation Envelope, the sustain level is properly expressed in percent of full scale.Because the volume envelope sustain level is expressed as an attenuation from full scale, the sustain level is analogously expressed as a decrease from full scale.A value of 0 indicates the sustain level is full level; this implies a zero duration of decay phase regardless of decay time.A positive value indicates a decay to the corresponding level.Values less than zero are to be interpreted as zero; values above 1000 are to be interpreted as 1000.  For example, a sustain level which corresponds to an absolute value 40 % of peak would be 600.
			releaseModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during release phase.For the Modulation Envelope, the release phase linearly ramps toward zero from the current level.If the current level were full scale, the Modulation Envelope Release Time would be the time spent in release phase until zero value were reached.A value of 0 indicates a 1 second decay time for a release from full level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a release time of 10 msec would be 1200log2(.01) = -7973.
			keynumToModEnvHold,		// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			keynumToModEnvDecay,	// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			delayVolEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Volume envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			attackVolEnv,			// This is the time, in absolute timecents, from the end of the Volume Envelope Delay Time until the point at which the Volume Envelope value reaches its peak.Note that the attack is �convex�; the curve is nominally such that when applied to the decibel volume parameter, the result is linear in amplitude.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
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
			startloopAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Startloop sample header parameter and the first sample data point to be repeated in this instrument�s loop.This parameter is added to the startloopAddrsOffset parameter.For example, if Startloop were 5, startloopAddrsOffset were 3 and startAddrsCoarseOffset were 2, the first sample data point in the loop would be sample data point 65544.
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
			endOper					// Unused, reserved.Should be ignored if encountered.Unique name provides value to end of defined list
		};

		union SFGenValue
		{
			signed short sword;			// signed 16 bit value
			unsigned short uword;		// unsigned 16 bit value
			struct
			{
				unsigned char lo;			// low value for ranges
				unsigned char hi;			// high value for ranges
			}
			range;
		};

		ref class SFGenAmount sealed
		{				// Generator amount structure
		internal:
			SFGenAmount() { val.sword = 0; }

			SFGenValue val;
		};

		// ���� ������� ����� ��� ������, ����� ���������, ����� �� �� �����
		// ���������� � ��� �������������
		//ref class SFSoundEngine;
		ref class SFVoice;
	}

	namespace Commands
	{
		public delegate bool CanExecuteMethod(Object^);


		// === ����������� �������-��������� ��� ����������� ������



		// ���������� ������ - ����������� ����� ������ ��������
		// void InsertBeat(Text, Position, Count?) // ������ Count ����� ������������ Position - �������� ���������
		// void DeleteBeat(Text, Position, Count?)

		public ref class CompositionHandlerArgs sealed
		{
			SketchMusic::Composition^ composition;	// TODO : �������� �� Composition
			String^ fileName;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class SymbolHandlerArgs sealed
		{
		public:
			SymbolHandlerArgs(Text^ text, PositionedSymbol^ sym)
			{
				_text = text;
				_sym = sym;
			}

			property Text^ _text;
			property PositionedSymbol^ _sym;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class SymbolPairHandlerArgs sealed
		{
		public:
			// ����������� ��� �������� � ����� ��������� (�����������, �������� � ���������)
			SymbolPairHandlerArgs(Text^ text, PositionedSymbol^ oldSym, PositionedSymbol^ newSym)
			{
				_text = text;
				_oldSym = oldSym;
				_newSym = newSym;
			}

			property Text^ _text;
			property PositionedSymbol^ _oldSym;
			property PositionedSymbol^ _newSym;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class MultiSymbolHandlerArgs sealed
		{
		public:
			// ����������� ��� �������� � ����� �������� (����������/��������)
			MultiSymbolHandlerArgs(Text^ text, IVector<PositionedSymbol^>^ sym)
			{
				_text = text;
				_OldSymbols = sym;
			}

			// ����������� ��� �������� � ����� ��������� (�����������, �������� � ���������)
			MultiSymbolHandlerArgs(Text^ text, IVector<PositionedSymbol^>^ oldSym, IVector<PositionedSymbol^>^ newSym)
			{
				_text = text;
				_OldSymbols = oldSym;
				_NewSymbols = newSym;
			}

			property Text^ _text;
			property IVector<PositionedSymbol^>^ _OldSymbols;
			property IVector<PositionedSymbol^>^ _NewSymbols;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		// �������/�������� �����
		public ref class BeatHandlerArgs sealed
		{
			Text^ text;
			Cursor^ position;
			int count;
		};

		// ����������� ������� �� ������ � �������
		public delegate bool Handler(Object^ args); // bool = ��������� ��� ���

		// ����������-��������
		// void Save(Text, FileName, Options?)
		// Text Load(FileName, Options?) -> void Load( ..Text^.. ) - ���������, ����� �� ���
		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public delegate bool CompositionHandler(CompositionHandlerArgs^ args);

		// ���������� ���������
		// void Insert(Text, PositionedSymbol)
		// void Move(Text, PositionedSymbol, Position newPos)
		// void Delete(Text, PositionedSymbol)
		// void Edit(Text, PositionedSymbol, Symbol newSym) // ������ ���������� delete-insert
		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public delegate bool SymbolHandler(SymbolHandlerArgs^ args);


		const int DEFAULT_MAX_COM = 100;

		// �������� ������
		partial ref class Command;
		partial ref class CommandState; // ������� ������ � � ����������
		partial ref class CommandManager; // ��������� ������� � ������ �������
	}

	namespace Player
	{
		// ��������� ������
		public enum class PlayerState
		{
			STOP = 0,	// ����� - ������
			PLAY = 1,	// �����������
			WAIT = 2,	// ��������������� - ����� ��� �������� �� ������� ���. ��� �� ������� ���������� ������
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class PlayerStateToPlayBtnContentConverter sealed : Windows::UI::Xaml::Data::IValueConverter
		{
		public:
			virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
			{
				SketchMusic::Player::PlayerState state = (SketchMusic::Player::PlayerState) value;
				switch (state)
				{
				case SketchMusic::Player::PlayerState::STOP:
					return "�������������";
				case SketchMusic::Player::PlayerState::PLAY:
				case SketchMusic::Player::PlayerState::WAIT:
					return "����������";
				}
				return nullptr;
			}

			virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
			{
				return nullptr;
			}

			PlayerStateToPlayBtnContentConverter() {}
		};


		ref class Player;
		interface class ISoundEngine;
		ref class SimpleSoundEngine;
		ref class SFSoundEngine;
		ref class SoundEnginePool;


		// ���������� � NoteVoicing ��� ���-������ ��� - ������ ������ ������� 
		// ��� ���������� ����������� ����� � �������� �������
		public ref class NoteOff sealed
		{
		private:
			cancellation_token_source source;
		internal:
			concurrency::cancellation_token GetToken()
			{
				return source.get_token();
			}
		public:
			NoteOff() {}

			void Cancel()
			{
				source.cancel();
			}
		};
	}
	
	/**
	 * ������������ ���, ������������ ������, ���������� �� ����������� �������������
	 * ���������� �������� ������������ ���������.
	 */
	namespace View
	{
		/**
		���������, ������������ ��������� ... ��������� ������
		*/
		public interface class IKeyPosition
		{
			Windows::Foundation::Point GetKeyPosition(int i);
		};

		public enum class KeyType
		{
			note			= 0,	// "������" ����
			relativeNote	= 1,	// �������������
			genericNote		= 2,	// "����������"
			octave			= 3,	// ����� �� ������
			space			= 4,	// ������, �� �� delete ?
			move			= 5,	// ������ left � right - �� ������� ���� �������� ����� ������������ �����
			enter			= 6,
			play			= 7,	// ����� ��������������� - ����, ����� ������������� - ������ ����
			stop			= 8,	// ��������� ������
			beat			= 9,
			shift			= 10,
			record			= 11,
			cycling			= 12,
			backspace		= 13,
			control			= 14,
			tempo			= 15,
			zoom			= 16,
			metronome		= 17,
			quantization	= 18,
			playGeneric		= 19,	// ���������� ���������� ����
			end				= 20,	// ����� ���� - ��������� ��������
			newPart			= 25,	// ������� � ������ ����� �����
			hide			= 30,	// �������� ����������
			layout			= 31,	// ������� ��������� ����������
			precount		= 32,	// ���������� ��������������� ���� ����� �������������
			accent			= 33,	// ��������������
			eraser			= 35,	// "������"
			scale			= 40,	// �����
			harmony			= 41,	// ������� ��������
			localHarmony	= 42,	// ��������� ������� ��������
		};

		public enum class KeyboardStateEnum
		{
			normal			= 0,
			pressed			= 1,	// ��� ������������ ��������� ������
			control			= 2,
			shift			= 3,
			octaveInc		= 4,
			octaveDec		= 5,
			play			= 6,	// ��� ����� ��������� ������ � ����������� ����������
			stop			= 7,	// ��� ����� ��������� ������ � ����������� ����������
			absoluteNote	= 10,	// ��� ����� ���������
			relativeNote	= 11,
			genericNote		= 12,
			specialSymbols	= 13,	// ������ �������, ������� � �.�.
			formatSymbols	= 14,
		};
		
		// �� ������ ������ ������� �������������� � �����
		// ���������� � ������� ����?
		public ref class KeyboardState sealed
		{
		public:
			property KeyboardStateEnum state;

			KeyboardState(KeyboardStateEnum s) { state = s; }
		};

		ref class Key;
		
		public ref class KeyboardEventArgs sealed
		{
		private:
			SketchMusic::View::Key^ m_key;

		public:
			property SketchMusic::View::Key^ key
			{
				SketchMusic::View::Key^ get() {
					return m_key;
				}
			}
			property int keysPressed;

			KeyboardEventArgs(SketchMusic::View::Key^ _key, int _keysPressed) : m_key(_key)
			{
				keysPressed = _keysPressed;
			}
		};

		// �� ������, ����� �� �������� ��������� �����, �� ���-�� ��� ������ ����
		// ���������� ����������� (���������� ����������, � ������������ ������������� ������ � �.�.) ����� �������������� ������ ������� ��������� ����������.
		// ������ ����� ����� �������������� ��� ����������� ����� (?) ���������
		public enum class KeyboardType
		{
			Basic		= 0,	// ������
			Relative	= 1,	// ���������������, ��� ��� ����� ��������� "������������" ��������, ���������� �������� ������� ����� ������������ "������������"
			Generic		= 2,	// ���������� ����
			Classic		= 4,	// ������������ ����������
			Modulators	= 5,	// "����������" ��� "�������������" ���. ������ ��������������, ��� ��� ����� �������� ���� ����� - ������� ������� �����
								// ��� {������� �������; ������� ��������� �����} - ��� ��������� ����� ������� - ������� ����� ���������� �������� ������	
								// ? ����� �������?
			Scale		= 6,
			Harmony		= 7,
			Controls	= 10,	// ���� ���� �� ������� 0
		};
		
		// ������������ ��������� ����� ����������� ���
		public enum class ViewType
		{
			TextRow		= 0,	// "�������" ������������� ���
			ChordView	= 1,	// ������������� ��� ����������������� �����
		};

		public interface class IKeyboard
		{
			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyPressed;	// ������� �� ���� �������
			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyReleased;	// ������� �� ���� �������
			event EventHandler<Windows::Foundation::Collections::IVector<SketchMusic::View::Key^>^>^ KeyboardReleased; // "����������" ���� ������
			event EventHandler<SketchMusic::View::KeyboardState^>^ KeyboardStateChanged;
		};

		// ���������� ��������
		ref class OnSymbolToStringConverter;
		ref class OnKeyToStringConverter;
		ref class TagToKeyConverter;		// ��� keytostring, ������ ��������. �� ����� ��� ����� ��� �� ��������� � �������� ������� ������������
		ref class BoolToColorConverter;
		ref class BlackKeyToColorConverter;
		ref class KeyStateToColorConverter;
		ref class OnPositionedSymbolToTextConverter;
		ref class PSymbolToVerticalPosConverter;
		ref class PSymbolToHorizontalPosConverter;
		ref class PSymbolToStyleConverter;
		ref class MultiplicatedLengthConverter;
		ref class PSymbolPosToTextConverter;
		ref class PartCatToTextConverter;
		ref class PartDynToTextConverter;
		ref class PartTimeToTextConverter;
		ref class PartNameToTextConverter;
		ref class TestData;
		
		// �������� ���������
		//ref class RadialKeyboard;
		ref class BaseKeyboard;

		ref class TextRow;			// "���������" ������������� ������������ ������
		ref class ChordView;		// ��� ����������� ���������� ���
		ref class CompositionView;	// ��� ����������� ���������� � �����
	}

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class ISymbolFactory sealed
	{
	public:
		static ISymbol^ Deserialize(Windows::Data::Json::JsonObject^ val);	// ������������� �����������
		static ISymbol^ CreateSymbol(SketchMusic::View::KeyType type, Object^ val);
	};
}