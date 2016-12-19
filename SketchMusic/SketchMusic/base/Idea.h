#pragma once

#include "../SketchMusic.h"
#include <string>
#include <vector>

//using namespace SketchMusic;


namespace SketchMusic
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Idea sealed
	{
	private:

	public:
		// ������������

		Idea() {}	// ������� ��� ������?
		//Idea(Platform::String^ _name) { Name = _name; }
		Idea(String^ _name, IdeaCategoryEnum _cat);
		//Idea(String^ _name, int _cat);
	
		// ��������

		property IdeaCategoryEnum Category;		// � ����� ��������� ��������� ����
		//property int Category;
		property Platform::String^ Name;		// ��� ��� ����
		// property Platform::String^ hash;		// ����� ? ���������� ��� ������ ����
		//property Platform::String^ Description;	// �������� ����
		//SketchMusic::Idea^	Parent;			// ������ �� ������������ ����.
		//property Platform::String^	Parent;		// ������ �� ������������ ���� � ���� ����
			// ������ �� ������� �� ����������� ������ ������� �� ��������-�������������� �������� �� �� (�� �� ���������� �� �������?)

			// �����, ��� � �� ���������, �� ��� �������, ��� ����� ������ ������� ����� ����� � �������� �����.
			// ����� ����� ����� ���������� ������ "��������" ����
			// TODO : "������������� ������������" ? - ���� ����� ���� �������� ����� ��� ���������� �����.
			// ������, ������ (2016.12.17) ��������������, ��� ����� ������� �������������� ������� "������� ����",
			// ������� ����� ��������� ������ ����� �� ����� �������� �����. �� ������ ������ � �������� ���� ����� ���������,
			// �� ��� ����� ��������

		/*
	internal:
	
		SketchMusic::Text^ Content;				// ����������, � ��� ���� �����������
		Windows::Foundation::Collections::IVector<String^>^ tags;		// ������ �����, �� ������� ����� ����� ������ ����
		Windows::Foundation::Collections::IVector<String^>^ projects;	// � ����� �������� ���� �������������
		DateTime creationTime;	// ����� � ���� �������� ����. ���������� ��� ���������� � ����������
		DateTime modifiedTime;	// ����� ��������� ����������� - �����?
		*/
	};
}