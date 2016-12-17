#pragma once

#include "../SketchMusic.h"

ref class SketchMusic::SFReader::SFData sealed
{
private:

internal:
	SFData();
	static SFData^ ReadSFData(Platform::String^ fileName);
	bool ReadInfo(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadSdta(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadPdta(Windows::Storage::Streams::DataReader^ dataReader);
	bool ReadIdta(Windows::Storage::Streams::DataReader^ dataReader);

	SFVersion^ version;
	SFVersion^ romver;
	unsigned int samplePos;
	unsigned int sampleSize;
	Platform::String^ name;
	//Platform::String^ fileName;
	Windows::Foundation::Collections::IVector<Platform::String^>^ info;
	Windows::Foundation::Collections::IVector<SFPreset^>^ presets;
	Windows::Foundation::Collections::IVector<SFInstrument^>^ instruments;
	Windows::Foundation::Collections::IVector<SFSample^>^ samples;
	Platform::Array<uint8>^ sdta;
};