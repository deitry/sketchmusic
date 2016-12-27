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
}