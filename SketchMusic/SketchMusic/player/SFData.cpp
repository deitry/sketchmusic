#include "pch.h"
#include "SFData.h"
#include "SoundFont.h"
#include "../SketchMusic.h"

using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

SketchMusic::SFReader::SFData::SFData()
{
	this->version = ref new SFVersion;
	this->romver = ref new SFVersion;
	this->samplePos = 0;
	this->sampleSize = 0;
	this->name = "";
	this->info = ref new Platform::Collections::Vector<Platform::String^>;
	this->presets = ref new Platform::Collections::Vector<SFPreset^>;
	this->instruments = ref new Platform::Collections::Vector<SFInstrument^>;
	this->samples = ref new Platform::Collections::Vector<SFSample^>;
	this->sdta = nullptr;	// инициализируется позже
}


SFData^ SketchMusic::SFReader::SFData::ReadSFData(Platform::String^ fileName)
{
	auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	auto path = folder->Path;
	auto fileName1 = "SketchMusic\\resources\\" + fileName;

	auto getData = create_task([=]() -> task < StorageFile^ >
	{
		return create_task(folder->GetFileAsync(fileName1));
	})
		.then([=](task<StorageFile^> fileOpenTask)->task < IRandomAccessStream^ >
	{
		StorageFile^ file = fileOpenTask.get();

		if (file == nullptr)
			throw ref new Exception(1, "Не удалось открыть файл");

		return create_task(file->OpenAsync(FileAccessMode::Read));
	})
		.then([=](task<IRandomAccessStream^> task) -> DataReader^
	{
		IRandomAccessStream^ readStream = task.get();
		UINT64 const size = readStream->Size;
		DataReader^ dataReader = ref new DataReader(readStream);
		dataReader->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
		//dataReader->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf16LE;

		auto load = create_task(dataReader->LoadAsync(static_cast<UINT32>(size)));
		load.wait();
		return dataReader;
	}).then([=](DataReader^ dataReader) -> SFData^
	{
		SFData^ sfData = ref new SFData;
		// вот тут надо разбить чтение файла на отдельные секции.
		// каждую можно представить в виде функции, возвращающей таск ... чтобы не в единой куче
		SFChunk^ riff = SFChunk::readChunk(dataReader, "RIFF", "sfbk");
		if (!riff)
			return nullptr;

		//if (fileSize != riff->size + 8)
		//	return;

		// 1 info
		sfData->ReadInfo(dataReader);

		// 2 sdta
		sfData->ReadSdta(dataReader);
		// 3 pdta
		sfData->ReadPdta(dataReader);
		sfData->ReadIdta(dataReader);

		return sfData;
	});
	try
	{
		return getData.get();
	}
	catch (Exception^ e)
	{
		return nullptr;
	}
}
#pragma endregion

bool SketchMusic::SFReader::SFData::ReadInfo(DataReader^ dataReader)
{
	SFChunk^ info = SFChunk::readChunk(dataReader, "LIST", "INFO");
	int size = info->size - 4;
	while (size > 0)
	{
		SFChunk^ chunk = SFChunk::readChunk(dataReader);

		if (chunk->name == "ifil")
		{
			if (chunk->size != 4)
				return false;

			this->version->wMajor = dataReader->ReadUInt16();
			this->version->wMinor = dataReader->ReadUInt16();

			// ошибка - не поддерживаемая версия
		}
		else if (chunk->name == "iver")
		{
			if (chunk->size != 4)
				return false;
			this->romver->wMajor = dataReader->ReadUInt16();
			this->romver->wMinor = dataReader->ReadUInt16();
		}
		else // всякие другие ид
		{
			// проверяем на валидность размера
			if ((chunk->name != "ICMT" && chunk->size > 256)
				|| (chunk->size > 65536)
				|| (chunk->size % 2))
				return false;

			// сохраняем в памяти
			Platform::String^ str = SketchMusic::SFReader::SFLoader::ReadString(dataReader, chunk->size);

			this->info->Append(str);
		}

		size -= chunk->size + 8;
	}
	return true;
}

bool SketchMusic::SFReader::SFData::ReadSdta(DataReader^ dataReader)
{
	SFChunk^ sdta = SFChunk::readChunk(dataReader, "LIST", "sdta");
	if (sdta->size != 0) // если имеются данные о семплах 
	{
		SFChunk^ smpl = SFChunk::readChunk(dataReader, "smpl");
		if (smpl->size <= sdta->size)
		{
			// если больше - 24-битный семпл, не поддерживается
			this->sdta = ref new Platform::Array<uint8>(smpl->size);
			dataReader->ReadBytes(this->sdta);
		}
	}
	return true;
}

bool SketchMusic::SFReader::SFData::ReadPdta(DataReader^ dataReader)
{
	SFChunk^ pdta = SFChunk::readChunk(dataReader, "LIST", "pdta");
	if (pdta)
	{
		// PHDR
		SFChunk^ phdr = SFChunk::readChunk(dataReader, "phdr");
		if (phdr && phdr->size && !(phdr->size % SFPHDRSIZE))
		{
			// загружаем пресеты
			SFPreset^ previous = nullptr;
			for (int i = phdr->size / SFPHDRSIZE; i > 0; i--) // -1
			{
				SFPreset^ preset = SFPreset::ReadPreset(dataReader);
				this->presets->Append(preset);

				if (previous)
					previous->zCnt = preset->zndx - previous->zndx;
				// если отрицательное - плохо
				previous = preset;
				// терминальный также записываем, чтобы сохранить его zndx
			}
			//SFPreset^ terminal = SFPreset::ReadPreset(dataReader); // последняя запись - не является пресетом
		} // phdr

		// PBAG
		SFChunk^ pbag = SFChunk::readChunk(dataReader, "pbag");
		if (pbag && pbag->size && !(pbag->size % SFBAGSIZE))
		{
			int size = pbag->size;

			SFPresetZone^ pZone = nullptr;
			for (auto preset : this->presets)
			{
				for (int i = 0; i < preset->zCnt; i++)
				{
					SFPresetZone^ zone = SFPresetZone::ReadZone(dataReader);
					preset->zones->Append(zone);
					size -= SFBAGSIZE;

					if (pZone)
					{
						pZone->modCnt = zone->modNdx - pZone->modNdx;
						pZone->genCnt = zone->genNdx - pZone->genNdx;
					}
					pZone = zone;
				}
			}

			if (size > 0)
			{
				// терминальная запись
				SFPresetZone^ zone = SFPresetZone::ReadZone(dataReader);
				if (pZone)
				{
					pZone->modCnt = zone->modNdx - pZone->modNdx;
					pZone->genCnt = zone->genNdx - pZone->genNdx;
				}
			}
		} // pbag

		SFChunk^ pmod = SFChunk::readChunk(dataReader, "pmod");
		if (pmod && pmod->size && !(pmod->size % SFMODSIZE))
		{
			int size = pmod->size;
			for (auto preset : this->presets)
			{
				for (auto zone : preset->zones)
				{
					// сравниваем индексы
					for (int i = 0; i < zone->modCnt; i++)
					{
						SFModulator^ mod = SFModulator::ReadModulator(dataReader);
						zone->modulators->Append(mod);

						size -= SFMODSIZE;
					}
				}
			}
			if (size > 0)
			{
				// терминальная запись
				SFModulator^ mod = SFModulator::ReadModulator(dataReader);
			}
		} // pmod

		// pgen
		SFChunk^ pgen = SFChunk::readChunk(dataReader, "pgen");
		if (pgen && pgen->size && !(pgen->size % SFGENSIZE))
		{
			int size = pgen->size;
			for (auto preset : this->presets)
			{
				for (auto zone : preset->zones)
				{
					// сравниваем индексы
					bool hasInstr = false;
					for (int i = 0; i < zone->genCnt; i++)
					{
						SFGenerator^ gen = SFGenerator::ReadGenerator(dataReader);
						size -= SFGENSIZE;
						if (!hasInstr)
						{
							zone->generators.insert(std::make_pair(gen->id, gen->amount));
						}
						// TODO : генераторы должны идти в определённом порядке
						// после инструмента не допускается никакого генератора
						if (gen->id == SFGeneratorID::instrument)
						{
							hasInstr = true;
							zone->instrNum = gen->amount->val.uword; // +1
						}
					}
					if (zone->generators.find(SFGeneratorID::instrument) == zone->generators.end())
					{
						// глобальная зона
						// если она не первая - переместить её в начало...
						// ну или вот так:
						preset->globalZone = zone;

						// TODO : если глобальных зон оказалось несколько, нужно разрулить
					}
				}
			}
			if (size > 0)
			{
				// терминальная запись
				SFGenerator^ gen = SFGenerator::ReadGenerator(dataReader);
			}
		} // pgen
	}
	return true;
}

bool SketchMusic::SFReader::SFData::ReadIdta(DataReader^ dataReader)
{
	// ihdr
	SFChunk^ ihdr = SFChunk::readChunk(dataReader, "inst");
	if (ihdr && ihdr->size && !(ihdr->size % SFIHDRSIZE))
	{
		// загружаем инструменты
		SFInstrument^ previous = nullptr;
		for (int i = ihdr->size / SFIHDRSIZE; i > 0; i--) // -1
		{
			SFInstrument^ instrument = SFInstrument::ReadInstrument(dataReader);
			this->instruments->Append(instrument);

			if (previous)
			{
				previous->zCnt = instrument->zNdx - previous->zNdx;
			}
			// если отрицательное - плохо
			previous = instrument;
			// терминальный также записываем, чтобы сохранить его zndx
		}
	} // ihdr

	// PBAG
	SFChunk^ ibag = SFChunk::readChunk(dataReader, "ibag");
	if (ibag && ibag->size && !(ibag->size % SFBAGSIZE))
	{
		int size = ibag->size;

		SFInstrumentZone^ pZone = nullptr;
		for (auto instrument : this->instruments)
		{
			for (int i = 0; i < instrument->zCnt; i++)
			{
				SFInstrumentZone^ zone = SFInstrumentZone::ReadZone(dataReader);
				instrument->zones->Append(zone);
				size -= SFBAGSIZE;

				if (pZone)
				{
					pZone->modCnt = zone->modNdx - pZone->modNdx;
					pZone->genCnt = zone->genNdx - pZone->genNdx;
				}
				pZone = zone;
			}
		}

		if (size > 0)
		{
			// терминальная запись
			SFInstrumentZone^ zone = SFInstrumentZone::ReadZone(dataReader);
			if (pZone)
			{
				pZone->modCnt = zone->modNdx - pZone->modNdx;
				pZone->genCnt = zone->genNdx - pZone->genNdx;
			}
		}
	} // ibag

	// imod
	SFChunk^ imod = SFChunk::readChunk(dataReader, "imod");
	if (imod && imod->size && !(imod->size % SFMODSIZE))
	{
		int size = imod->size;
		for (auto instrument : this->instruments)
		{
			for (auto zone : instrument->zones)
			{
				// сравниваем индексы
				for (int i = 0; i < zone->modCnt; i++)
				{
					SFModulator^ mod = SFModulator::ReadModulator(dataReader);
					zone->modulators->Append(mod);

					size -= SFMODSIZE;
				}
			}
		}
		if (size > 0)
		{
			// терминальная запись
			SFModulator^ mod = SFModulator::ReadModulator(dataReader);
		}
	} // imod

	// igen
	SFChunk^ igen = SFChunk::readChunk(dataReader, "igen");
	if (igen && igen->size && !(igen->size % SFGENSIZE))
	{
		int size = igen->size;
		for (auto instrument : this->instruments)
		{
			for (auto zone : instrument->zones)
			{
				// сравниваем индексы
				bool hasSample = false;
				for (int i = 0; i < zone->genCnt; i++)
				{
					SFGenerator^ gen = SFGenerator::ReadGenerator(dataReader);
					if (!hasSample)
					{
						zone->generators.insert(std::make_pair(gen->id, gen->amount));
					}
					// TODO : генераторы должны идти в определённом порядке
					// после сэмпла не допускается никакого генератора
					if (gen->id == SFGeneratorID::sampleID)
					{
						hasSample = true;
						zone->sampleNum = gen->amount->val.uword; // +1
					}

					size -= SFGENSIZE;
				}
				if (zone->generators.find(SFGeneratorID::sampleID) == zone->generators.end())
				{
					// глобальная зона
					// если она не первая - переместить её в начало...
					// ну или вот так:
					instrument->globalZone = zone;

					// TODO : если глобальных зон оказалось несколько, нужно разрулить
				}
			}
		}
		if (size > 0)
		{
			// терминальная запись
			SFGenerator^ gen = SFGenerator::ReadGenerator(dataReader);
		}
	} // igen

	// shdr
	SFChunk^ shdr = SFChunk::readChunk(dataReader, "shdr");
	if (shdr && shdr->size && !(shdr->size % SFSHDRSIZE))
	{
		// за исключением терминального
		for (int cnt = shdr->size / SFSHDRSIZE - 1; cnt > 0; cnt--)
		{
			SFSample^ sample = SFSample::ReadSample(dataReader);
			this->samples->Append(sample);
		}
		SFSample^ terminal = SFSample::ReadSample(dataReader); // терминальный
	}

	// fixup pgen 
	for (auto preset : this->presets)
	{
		for (auto zone : preset->zones)
		{
			zone->instrument = this->instruments->GetAt(zone->instrNum);
			for (auto gen : zone->generators)
			{
				switch (gen.first)
				{
				case SketchMusic::SFReader::SFGeneratorID::keyRange:
					zone->keyHi = gen.second->val.range.hi;
					zone->keyLo = gen.second->val.range.lo;
					break;
				case SketchMusic::SFReader::SFGeneratorID::velRange:
					zone->velHi = gen.second->val.range.hi;
					zone->velLo = gen.second->val.range.lo;
					break;
				}
			}
		}
	}

	// fixup igen
	// связывание сэмпла с зоной инструмента
	for (auto instrument : this->instruments)
	{
		for (auto zone : instrument->zones)
		{
			zone->sample = this->samples->GetAt(zone->sampleNum);
			for (auto gen : zone->generators)
			{
				switch (gen.first)
				{
				case SketchMusic::SFReader::SFGeneratorID::keyRange:
					zone->keyHi = gen.second->val.range.hi;
					zone->keyLo = gen.second->val.range.lo;
					break;
				case SketchMusic::SFReader::SFGeneratorID::velRange:
					zone->velHi = gen.second->val.range.hi;
					zone->velLo = gen.second->val.range.lo;
					break;
				case SketchMusic::SFReader::SFGeneratorID::overridingRootKey:
					if ((gen.second->val.sword >= 0) && (gen.second->val.sword <= 127))
					{
						zone->sample->origPitch = gen.second->val.sword;
					}
					break;
				case SketchMusic::SFReader::SFGeneratorID::sampleModes:
					zone->sampleMode = gen.second->val.uword;
					break;
				}
			}
		}
	}

	// fixup sample
	// конвертация меток о конце семпла в смещение относительно начала... оно надо?
	for (auto sample : this->samples)
	{
		sample->end -= sample->start;
		sample->loopEnd -= sample->loopStart;
		
		//sample->data = new unsigned char[sample->end + 1];
		//unsigned char* startData = this->sdta->begin() + sample->start;
		//memcpy(sample->data, startData, sample->end);
		//sample->data[sample->end] = '\0';
		sample->data = &(this->sdta->begin()[sample->start]);
	}
	return true;
}