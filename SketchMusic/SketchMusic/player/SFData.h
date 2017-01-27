#pragma once

#include "../SketchMusic.h"
#include "SoundFont.h"

public ref class SketchMusic::SFReader::SFData sealed
{
private:

internal:
	SFData();
	static SFData^ ReadSFData(Platform::String^ fileName);
	bool ReadInfo(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadSdta(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadPdta(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadIdta(Windows::Storage::Streams::DataReader^ dataReader);
	
	// Упрощённая функция, чтобы считывать названия инструментов без чтения всего файла
	// А может и весь файл стоит читать, чтобы иметь возможность превью
	//static SFData^ GetInstrumentsList(Platform::String^ fileName);

	SFVersion^ version;
	SFVersion^ romver;
	unsigned int samplePos;
	unsigned int sampleSize;
	Platform::String^ name;
	//Platform::String^ fileName;
	
	Platform::Array<uint8>^ sdta;
	Windows::Foundation::Collections::IObservableVector<Platform::String^>^ info;
	Windows::Foundation::Collections::IObservableVector<SFInstrument^>^ instruments;
	Windows::Foundation::Collections::IObservableVector<SFSample^>^ samples;
public:
	property Windows::Foundation::Collections::IObservableVector<SFPreset^>^ presets;
	
};