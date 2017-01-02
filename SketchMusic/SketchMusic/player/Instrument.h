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
		Instrument(Platform::String^ name) { _name = name; }
		Instrument(Platform::String^ name, Platform::String^ pset) { _name = name; preset = pset; }

		bool EQ(SketchMusic::Instrument^ instr) { if ((this->_name == instr->_name) && (this->preset == instr->preset)) { return true; } else { return false; } }

		property Platform::String^ _name;	// название инструмента
		property Platform::String^ preset;	// номер пресета - для выбора в sf2
	};

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