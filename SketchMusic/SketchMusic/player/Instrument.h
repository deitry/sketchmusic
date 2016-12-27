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

		property Platform::String^ _name;		// название инструмента
		// property Platform::String^ category;

		//void addSample(Platform::String^ sampleName);
		//void deleteSample(Platform::String^ sampleName);	
		//Windows::Foundation::Collections::IVector<Platform::String^>^ getSamples();
	};
}