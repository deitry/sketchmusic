#pragma once

#include "../SketchMusic.h"
#include <string>
#include <vector>

using namespace Windows::UI::Xaml::Data;

namespace SketchMusic
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Idea sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
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
		property long long ModifiedTime;
		// ������������ ����� INotifyPropertyChanged
		virtual event PropertyChangedEventHandler ^ PropertyChanged;
		void RaisePropertyChanged() { PropertyChanged(this, ref new PropertyChangedEventArgs("")); }
		// ����� ��������� ����������� - �����?
	};


	public ref class TimeToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			long long time = (long long)value;
			struct tm t;
			_localtime64_s(&t, &time);
			t.tm_year += 1900;
			t.tm_mon += 1;
			//t.tm_mday -= 1;
			//t.tm_hour -= 4;
			//printf_s("%04i/%02i/%02i %02i:%02i:%02i\n\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
			// TODO : ������� ����� ��������������
			Platform::String^ str = "" + t.tm_year + "." + t.tm_mon + "." + t.tm_mday + " " +
				t.tm_hour;
			str += ":" + ((t.tm_min < 10) ? "0" : "") + t.tm_min + ":" +
						 ((t.tm_sec<10) ? "0" : "") + t.tm_sec;
			return str;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return 0;
		}

		TimeToStrConverter() {}
	};
}