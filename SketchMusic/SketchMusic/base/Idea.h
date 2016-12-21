#pragma once

#include "../SketchMusic.h"
#include <string>
#include <vector>

namespace SketchMusic
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Idea sealed
	{
	public:
		// ������������

		//Idea(Platform::String^ _name) { Name = _name; }
		Idea();
		Idea(String^ _name, IdeaCategoryEnum _cat);
		Idea(int _hash, String^ _name, IdeaCategoryEnum _cat, String^ _desc, int _parent, byte _rating,
			 SketchMusic::Text^ _content, String^ tags, String^ projects, long long _created, long long _modified); // ��� ��������
		Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
			Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified);	// ����� �� ����������������� �������

		SketchMusic::Text^ GetContent();

		// ��������

		property int Hash;						// ���������� ��� ������ ����
		property IdeaCategoryEnum Category;		// � ����� ��������� ��������� ����
		property Platform::String^ Name;		// ��� ��� ����
		property Platform::String^ Description;	// �������� ����
		//SketchMusic::Idea^	Parent;			// ������ �� ������������ ����.
		property int Parent;					// ��� ������������ ����
		property byte Rating;					// ������, ������ ������ ����
		
			// ������ �� ������� �� ����������� ������ ������� �� ��������-�������������� �������� �� �� (�� �� ���������� �� �������?)

			// �����, ��� � �� ���������, �� ��� �������, ��� ����� ������ ������� ����� ����� � �������� �����.
			// ����� ����� ����� ���������� ������ "��������" ����
			// TODO : "������������� ������������" ? - ���� ����� ���� �������� ����� ��� ���������� �����.
			// ������, ������ (2016.12.17) ��������������, ��� ����� ������� �������������� ������� "������� ����",
			// ������� ����� ��������� ������ ����� �� ����� �������� �����. �� ������ ������ � �������� ���� ����� ���������,
			// �� ��� ����� ��������

		property SketchMusic::Text^ Content;	// ����������, � ��� ���� �����������
		property Platform::String^ SerializedContent;	// ���� ���� �� �� ������������� �����, 
			// ��� ����������� ������ ��������� �� ����� �������. ����� �� ������� �����, ���������� ����� �������
			// � ��������������� ����, ���� �� ����������� �������.
		property Platform::String^ Tags;		// ������ �����, �� ������� ����� ����� ������ ����
		//Windows::Foundation::Collections::IVector<String^>^
		property Platform::String^ Projects;	// � ����� �������� ���� �������������
		property long long CreationTime;		// ����� � ���� �������� ����. ���������� ��� ���������� � ����������
		property long long ModifiedTime;		// ����� ��������� ����������� - �����?
	};
}