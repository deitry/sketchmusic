#pragma once

/**
Определения классов, подходящих для модели синтеза SoundFont
*/


#include "../SketchMusic.h"

using namespace SketchMusic::SFReader;

#define fccRIFF "FFIR"

#define GEN_LAST 1	// надо выяснить, что это за константа и чему она равна, а пока пусть будет 1
// это в смысле максимальное значение в перечислении?

ref class SketchMusic::SFReader::SoundFont sealed
{
internal:
	SoundFont(Platform::String^ fileName);
	
	property Platform::String^ name;

	unsigned int samplePos;
	unsigned int sampleSize;
	short* sampleData;

	Windows::Foundation::Collections::IVector<SFSample^>^ samples;
	Windows::Foundation::Collections::IVector<SFPreset^>^ presets;
};

ref class SketchMusic::SFReader::SFVersion sealed
{
internal:
	WORD wMajor;
	WORD wMinor;
};

/* sfont file chunk sizes */
#define SFPHDRSIZE	38
#define SFBAGSIZE	4
#define SFMODSIZE	10
#define SFGENSIZE	4
#define SFIHDRSIZE	22
#define SFSHDRSIZE	46

ref class SketchMusic::SFReader::SFChunk sealed
{
internal:
	Platform::String^ name;
	unsigned int size;
	Platform::String^ id;

	static SFChunk^ readChunk(Windows::Storage::Streams::DataReader^ dataReader, 
							  Platform::String^ name = nullptr, 
							  Platform::String^ id = nullptr)
	{
		SFChunk^ chunk = ref new SFChunk;
		chunk->name = dataReader->ReadString(4);
		if (name && (chunk->name != name))
			return nullptr;
		// TODO : удалять созданный чанк

		chunk->size = dataReader->ReadInt32();

		
		// считываем id, если ожидаем его здесь увидеть. Иначе ожидаем данные...
		if (id)
		{
			chunk->id = dataReader->ReadString(4);
			if (id != chunk->id)
				return nullptr;
		}

		return chunk;
	}
};

ref class SketchMusic::SFReader::SFLoader sealed
{
	Windows::Storage::StorageFile^ file;
	Windows::Storage::Streams::DataReader^ reader;

	void LoadBody();	// позже можно будет привести в божеский вид
		// предположительно часть, а то и все функции стоит перенести в новый класс SFLoader()
	void GetInfo(SFData^ sfData);	// передаётся заранее известный размер чанка
	void GetSdta(SFData^ sfData);	// может ещё и возвращать sfData для наглядности?
	void GetPdta(SFData^ sfData);

	int ReadId();
	
public:
	SFLoader(Platform::String^ fileName);
	
	static Platform::String^ ReadString(Windows::Storage::Streams::DataReader^ dataReader, int size)
	{
		Platform::Array<unsigned char>^ bytes = ref new Platform::Array<unsigned char>(size);
		dataReader->ReadBytes(bytes);
		bytes->Data[size - 1] = '\0';	// получается, что обрезаем последний символ
		wchar_t* dest = new wchar_t[size];
		//for (int i = 0; i < size+2; i++)
		//	dest[i] = '\0';
	
		size_t max = size;
		size_t conv = 0;
		auto er = mbstowcs_s(&conv, dest, max, (char*)(bytes->Data), max);
		
		Platform::String^ str = ref new Platform::String(dest);//dataReader->ReadString(chunk->size);
		delete dest;
		delete bytes;
		return str;
	}

	SFData^ GetData();
	void LoadSampleData(SFData^ sfData); // как-то это странно. Надо будет уйти от наследия С
};

ref class SketchMusic::SFReader::SFModulator sealed
{
internal:
	static SFModulator^ ReadModulator(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFModulator^ mod = ref new SFModulator;
		mod->src = dataReader->ReadUInt16();
		mod->dest = dataReader->ReadUInt16();
		mod->amount = dataReader->ReadUInt16();
		mod->amtsrc = dataReader->ReadUInt16();
		mod->trans = dataReader->ReadUInt16();

		return mod;
	}

	unsigned short src;
	unsigned short dest;
	unsigned short amount;
	unsigned short amtsrc;
	unsigned short trans;


	bool EQ(SFModulator^ mod)
	{
		return false;
	}
};

ref class SketchMusic::SFReader::SFPresetZone sealed
{
internal:
	static SFPresetZone^ ReadZone(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFPresetZone^ zone = ref new SFPresetZone;
		zone->genNdx = dataReader->ReadUInt16();
		zone->modNdx = dataReader->ReadUInt16();
		zone->instrNum = 0;

		//zone->generators = ref new Platform::Collections::Map < SFGeneratorID, SFGenAmount^ >;
		zone->modulators = ref new Platform::Collections::Vector < SFModulator^ >;
		return zone;
	}

	property Platform::String^ name;
	SFInstrument^ instrument;
	int keyLo;
	int keyHi;
	int velLo;
	int velHi;
	unsigned short genNdx;
	unsigned short modNdx;
	unsigned short genCnt;
	unsigned short modCnt;

	unsigned short instrNum;

	//Windows::Foundation::Collections::IMap<SFGeneratorID, SFGenAmount^>^ generators;
	Windows::Foundation::Collections::IVector<SFModulator^>^ modulators;
	std::map<SFGeneratorID, SFGenAmount^> generators;

	bool inZone(int key, int vel)
	{
		if ((keyLo || keyHi) &&
			((key < keyLo) || (key > keyHi))) return false;
		if (vel != -1)
		{
			if ((velLo || velHi) &&
				((vel < velLo) || (vel > velHi))) return false;
		}
		return true;
	}
};

ref class SketchMusic::SFReader::SFPreset sealed
{
internal:
	static SFPreset^ ReadPreset(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFPreset^ preset = ref new SFPreset;
		preset->name = SketchMusic::SFReader::SFLoader::ReadString(dataReader, 20);
		preset->number = dataReader->ReadUInt16();
		preset->bank = dataReader->ReadUInt16();
		preset->zndx = dataReader->ReadUInt16();
		preset->library = dataReader->ReadUInt32();
		preset->genre = dataReader->ReadUInt32();
		preset->morph = dataReader->ReadUInt32();
		preset->zones = ref new Platform::Collections::Vector < SFPresetZone^ > ;
		return preset;
	}

	//SketchMusic::SoundFont^ parent;
	Platform::String^ name;
	unsigned short bank;
	unsigned short number;
	unsigned short zndx;	// индекс пресета
	unsigned short zCnt;	// посчитанное количество зон
	unsigned int library;
	unsigned int genre;
	unsigned int morph;
	SFPresetZone^ globalZone;

	Windows::Foundation::Collections::IVector<SFPresetZone^>^ zones;

	void noteOn(int key, int vel);
	void updateParams();
};

ref class SketchMusic::SFReader::SFInstrumentZone sealed
{
internal:
	static SFInstrumentZone^ ReadZone(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFInstrumentZone^ zone = ref new SFInstrumentZone;
		zone->genNdx = dataReader->ReadUInt16();
		zone->modNdx = dataReader->ReadUInt16();
		zone->sampleNum = 0;
		zone->modulators = ref new Platform::Collections::Vector < SFModulator^ > ;
		return zone;
	}

	Platform::String^ name;
	SFSample^ sample;
	int keyLo;
	int keyHi;
	int velLo;
	int velHi;
	unsigned short genNdx;
	unsigned short modNdx;
	unsigned short genCnt;
	unsigned short modCnt;

	unsigned short sampleNum;

	//Windows::Foundation::Collections::IMap<SFGeneratorID, SFGenAmount^>^ generators;
	std::map<SFGeneratorID, SFGenAmount^> generators;
	Windows::Foundation::Collections::IVector<SFModulator^>^ modulators;

	bool inZone(int key, int vel = -1)
	{
		if (//(keyLo || keyHi) &&
			((key < keyLo) || (key > keyHi))) return false;
		if (vel != -1)
		{
			if ((velLo || velHi) &&
				((vel < velLo) || (vel > velHi))) return false;
		}
		return true;
	}
};

ref class SketchMusic::SFReader::SFInstrument sealed
{
internal:
	static SFInstrument^ ReadInstrument(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFInstrument^ instrument = ref new SFInstrument;
		instrument->name = SketchMusic::SFReader::SFLoader::ReadString(dataReader, 20);
		instrument->zNdx = dataReader->ReadUInt16();
		instrument->zones = ref new Platform::Collections::Vector < SFInstrumentZone^ > ;
		return instrument;
	}

	property Platform::String^ name;
	unsigned short zNdx;
	unsigned short zCnt;
	SFInstrumentZone^ globalZone;
	property Windows::Foundation::Collections::IVector<SFInstrumentZone^>^ zones;
};

ref class SketchMusic::SFReader::SFGenerator sealed
{
internal:
	SFGenerator()
	{
		this->amount = ref new SFGenAmount;
	}

	static SFGenerator^ ReadGenerator(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFGenerator^ gen = ref new SFGenerator;
		gen->id = static_cast<SFGeneratorID>( dataReader->ReadUInt16());
		int level = 0;	// нужен для упорядочивания генераторов\
		// - некоторые из них могут следовать друг за другом только в определённом порядке
		if ((gen->id == SFGeneratorID::keyRange) || (gen->id == SFGeneratorID::velRange))
		{
			gen->amount->val.range.lo = dataReader->ReadByte();
			gen->amount->val.range.hi = dataReader->ReadByte();
		}
		else if (gen->id == SFGeneratorID::instrument)
		{
			gen->amount->val.uword = dataReader->ReadUInt16();
		}
		else
		{
			gen->amount->val.sword = dataReader->ReadInt16();
		}

		return gen;
	}

	SFGeneratorID id;
	SFGenAmount^ amount;
	
	//unsigned int i;		// судя по всему, количество генераторов фиксированно
		// тогда, возможно, следует несколько по-другому их организовать.
	//unsigned int flags;
	//unsigned int value;

	/* SF 2.01 section 8.5 page 58: If some generators are
	 * encountered at preset level, they should be ignored */
	static bool ignoreAtPresetLevel(SFGenerator^)
	{
		// список тех, которые надо игнорить
		/*if ((i != GEN_STARTADDROFS)
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
		&& (i != GEN_OVERRIDEROOTKEY)) {*/

		return false;
	}
};

/**
Пока будет в себе инкапсулировать работу с генераторами и модуляторами.
В дальнейшем возможно появятся новые классы голосов, позволяющие или не позволяющие 
работать с генераторами и модуляторами.. например SFVoice - всё позволяет, SimpleVoice - ничего не позволяет
SinVoice - голос со программно вшитым сэмплом - чтобы не зависеть от отдельных файлов...

хз, надо подумать.
*/
ref class SketchMusic::SFReader::SFVoice sealed
{
	/*
	unsigned int id;	// the id is incremented for every new noteon.
		//it's used for noteoff's
	unsigned char status;
	unsigned char chan;	// the channel number, quick access for channel messages
	unsigned char key;	// the key, quick access for noteoff
	unsigned char vel;	//the velocity
	fluid_channel_t* channel;
	fluid_gen_t gen[GEN_LAST];
	fluid_mod_t mod[FLUID_NUM_MOD];
	int mod_count;
	fluid_sample_t* sample;	// Pointer to sample (dupe in rvoice)

	int has_noteoff;	// Flag set when noteoff has been sent

	// basic parameters
	fluid_real_t output_rate;	// the sample rate of the synthesizer (dupe in rvoice)

	unsigned int start_time;
	fluid_adsr_env_t volenv;	// Volume envelope (dupe in rvoice)

	// basic parameters
	fluid_real_t pitch;	// the pitch in midicents (dupe in rvoice)
	fluid_real_t attenuation;	// the attenuation in centibels (dupe in rvoice)
	fluid_real_t root_pitch;

	// master gain (dupe in rvoice)
	fluid_real_t synth_gain;

	// pan
	fluid_real_t pan;
	fluid_real_t amp_left;
	fluid_real_t amp_right;

	// reverb
	fluid_real_t reverb_send;
	fluid_real_t amp_reverb;

	// chorus
	fluid_real_t chorus_send;
	fluid_real_t amp_chorus;

	// rvoice control
	fluid_rvoice_t* rvoice;
	fluid_rvoice_t* overflow_rvoice; // Used temporarily and only in overflow situations
	int can_access_rvoice; // False if rvoice is being rendered in separate thread
	int can_access_overflow_rvoice; // False if overflow_rvoice is being rendered in separate thread

	// for debugging
	int debug;
	double ref;
	*/
	
	Windows::Foundation::Collections::IVector<SFGenerator^>^ generators;
	Windows::Foundation::Collections::IVector<SFModulator^>^ modulators;

public:

	void start() {}

	void setGenerator(SFGenerator^ gen)
	{
		/*voice->gen[i].val = val;
		voice->gen[i].flags = GEN_SET;
		if (i == GEN_SAMPLEMODE)
			UPDATE_RVOICE_I1(fluid_rvoice_set_samplemode, val);
		*/
	}

	void addModulator(SFModulator^ mod)
	{
		// если такой модулятор уже есть, то он должен быть удалён
		for (int i = 0; i < modulators->Size; i++)
		{
			if (mod->EQ(modulators->GetAt(i)))
			{
				modulators->RemoveAt(i);
					// не самый эффективный путь.
					// Может, придётся вернуться к тому же, что было во флюид - 
					// добавление только определённых модуляторов.
					// Хотя вряд ли будет слишком сильно эффективно.
			}
			else
			{
				modulators->Append(mod);
			}
		}
	}

	/**
	* Offset the value of a generator.
	* @param voice Voice instance
	* @param i Generator ID (#fluid_gen_type)
	* @param val Value to add to the existing value
	*/
	/*void
		fluid_voice_gen_incr(fluid_voice_t* voice, int i, float val)
	{
		voice->gen[i].val += val;
		voice->gen[i].flags = GEN_SET;
	}*/

	void genIncr(SFGenerator^ gen)
	{

	}
};

ref class SketchMusic::SFReader::SFSample sealed
{
internal:
	static SFSample^ ReadSample(Windows::Storage::Streams::DataReader^ dataReader)
	{
		SFSample^ sample = ref new SFSample;
		sample->name = SketchMusic::SFReader::SFLoader::ReadString(dataReader, 20);
		sample->start = dataReader->ReadUInt32();
		sample->end = dataReader->ReadUInt32();
		sample->loopStart = dataReader->ReadUInt32();
		sample->loopEnd = dataReader->ReadUInt32();
		sample->sampleRate = dataReader->ReadUInt32();
		sample->origPitch = dataReader->ReadByte();
		sample->pitchAdj = dataReader->ReadByte();
		sample->link = dataReader->ReadUInt16();
		sample->sampleType = dataReader->ReadUInt16();
		sample->data = nullptr;

		return sample;
	}

	Platform::String^ name;
	unsigned int start;
	unsigned int end;
	unsigned int loopStart;
	unsigned int loopEnd;
	unsigned int sampleRate;
	unsigned char origPitch;
	unsigned char pitchAdj;
	unsigned short link;
	unsigned short sampleType;
	unsigned char* data;

private:
	~SFSample()
	{
		//if (data)
		//	delete data;
	}

};