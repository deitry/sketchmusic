#pragma once

#include <string>
#include <vector>

#include "../SketchMusic.h"

/**
���������� ����������.
����� �������� �� ���������� �������.
����� ������������ ������ � ����� �������� ������
*/
namespace SketchMusic
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Instrument sealed
	{
	private:
		//std::vector<Platform::String^> _samples;	// ����� �������
		// ����������� � ���� ��������. ���������� ���������� ����� ������������ (���������?) SoundEngine
		// ���������� ����� ������ ������� �������� ��� �������, ������� ��� �����.
	public:
		Instrument() {}
		Instrument(Platform::String^ name) { _name = name; }

		property Platform::String^ _name;		// �������� �����������
		// property Platform::String^ category;

		//void addSample(Platform::String^ sampleName);
		//void deleteSample(Platform::String^ sampleName);	
		//Windows::Foundation::Collections::IVector<Platform::String^>^ getSamples();
	};

	public ref class InstrumentToTextConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			SketchMusic::Instrument^ instr = dynamic_cast<SketchMusic::Instrument^>(value);
			if (instr == nullptr) return "";

			return instr->_name;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		InstrumentToTextConverter() {}
	};
}