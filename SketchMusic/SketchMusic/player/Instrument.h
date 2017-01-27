#pragma once

#include <string>
#include <vector>

#include "../SketchMusic.h"

/**
Определяет инструмент.
Может состоять из нескольких семплов.
Будет передаваться вместе с одной дорожкой текста
*/
namespace SketchMusic
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Instrument sealed
	{
	private:
		//std::vector<Platform::String^> _samples;	// набор семплов
		// представлен в виде названий. Конкретные экземпляры будет подхватывать (создавать?) SoundEngine
		// Инструмент будет только хранить названия тех семплов, которые ему нужны.
	public:
		Instrument() {}
		Instrument(Platform::String^ name) { _name = name; FileName = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + "\\SketchMusic\\resources\\" + name; }
		//Instrument(Platform::String^ name, Platform::String^ pset) { _name = name; preset = pset; }
		Instrument(Platform::String^ name, Platform::String^ fname, Platform::String^ pset)
		{ 
			_name = name; 
			if (fname && fname != "") 
				FileName = fname;
			else FileName = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + "\\SketchMusic\\resources\\" + name;
			preset = pset; 
		}
		Instrument(SketchMusic::Instrument^ instr) 
		{ 
			_name = instr->_name; 
			preset = instr->preset; 
			if (instr->FileName && instr->FileName != "")
				FileName = instr->FileName; 
			else FileName = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + "\\SketchMusic\\resources\\" + instr->_name;
		}

		Windows::Data::Json::IJsonValue^ Serialize()
		{
			namespace t = SketchMusic::SerializationTokens;
			using namespace Windows::Data::Json;
			auto json = ref new Windows::Data::Json::JsonObject;
			json->Insert(t::INSTR, JsonValue::CreateStringValue(_name));
			json->Insert(t::INSTR_FNAME, JsonValue::CreateStringValue(FileName));
			json->Insert(t::INSTR_PRESET, JsonValue::CreateStringValue(preset));
			return json;
		}

		static Instrument^ Deserialize(Windows::Data::Json::JsonObject^ json)
		{
			namespace t = SketchMusic::SerializationTokens;
			using namespace Windows::Data::Json;
			//auto json = ref new Windows::Data::Json::JsonObject;
			auto instr = ref new Instrument;
			try
			{
				instr->_name = json->GetNamedString(t::INSTR);
			}
			catch (...)
			{
				return nullptr;
			}
			
			instr->FileName = json->GetNamedString(t::INSTR_FNAME);
			instr->preset = json->GetNamedString(t::INSTR_PRESET);

			return instr;
		}

		bool EQ(SketchMusic::Instrument^ instr) { if ((this->_name == instr->_name) && (this->preset == instr->preset)) { return true; } else { return false; } }

		property Platform::String^ _name;	// название инструмента
		property Platform::String^ preset;	// номер пресета - для выбора в sf2
		property Platform::String^ FileName;	// имя файла, соответствующего данному инструменту - включает в себя путь до файла
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class InstrumentToTextConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			SketchMusic::Instrument^ instr = dynamic_cast<SketchMusic::Instrument^>(value);
			if (instr == nullptr) return "";

			return instr->_name + " : " + instr->preset;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		InstrumentToTextConverter() {}
	};
}