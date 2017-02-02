#pragma once

#include "../SketchMusic.h"
#include "CompositionData.h"
#include <string>
#include <vector>

using namespace Windows::UI::Xaml::Data;
using namespace Platform;
using namespace SketchMusic;

namespace SketchMusic
{
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class Idea sealed
	{
	public:
		// ������������

		//Idea(Platform::String^ _name) { Name = _name; }
		Idea();
		Idea(String^ _name, IdeaCategoryEnum _cat);
		Idea(int _hash, String^ _name, IdeaCategoryEnum _cat, String^ _desc, int _parent, byte _rating,
			CompositionData^ _content, String^ tags, String^ projects, long long _created, long long _modified); // ��� ��������
		Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
			Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified);	// ����� �� ����������������� �������

		CompositionData^ GetContent();

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

		property CompositionData^ Content;	// ����������, � ��� ���� �����������
		property Platform::String^ SerializedContent;	// ���� ���� �� �� ������������� �����, 
			// ��� ����������� ������ ��������� �� ����� �������. ����� �� ������� �����, ���������� ����� �������
			// � ��������������� ����, ���� �� ����������� �������.
		property Platform::String^ Tags;		// ������ �����, �� ������� ����� ����� ������ ����
		//Windows::Foundation::Collections::IVector<String^>^
		property Platform::String^ Projects;	// � ����� �������� ���� �������������
		property long long CreationTime;		// ����� � ���� �������� ����. ���������� ��� ���������� � ����������
		property long long ModifiedTime;

		// ������������ ����� INotifyPropertyChanged
		//virtual event PropertyChangedEventHandler ^ PropertyChanged;
		//void RaisePropertyChanged() { PropertyChanged(this, ref new PropertyChangedEventArgs("")); }

		// ����� ��������� ����������� - �����?
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
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
			Platform::String^ str = "" + t.tm_year + "." +
				((t.tm_mon < 10) ? "0" : "") + t.tm_mon + "." +
				((t.tm_mday < 10) ? "0" : "") + t.tm_mday + " " +
				t.tm_hour;
			str += ":" + ((t.tm_min < 10) ? "0" : "") + t.tm_min + ":" +
				((t.tm_sec < 10) ? "0" : "") + t.tm_sec;
			return str;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return (int)0;
		}

		TimeToStrConverter() {}
	};

	// ����� ��� ���������� ���� � ������. ���������� PositionedSymbol ����� ����� �������, �� ������ ����� �������� ��� �� �����
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class PositionedIdea sealed
	{
	private:
		PositionedIdea() {}
	public:
		// ������ ������, ���� �� �� ����� ������������ �����-���� ������ ������������� �
		// � ����������� ������� ������ ����� ������ �������� ������ ����.

		property SketchMusic::Cursor^ Pos;		// �������� ���������
		property SketchMusic::Cursor^ Length;	// �����. ������� ������� �������������� ��� Pos + Length
		property int Layer;						// ����, �� ������� ������������� ����

		property SketchMusic::Idea^ Content;		// ��������� ����

		PositionedIdea(SketchMusic::Cursor^ pos, SketchMusic::Cursor^ length, int layer, SketchMusic::Idea^ idea)
		{
			Pos = pos; Length = length; Layer = layer; Content = idea;
		}
		PositionedIdea(SketchMusic::Cursor^ pos, int layer, SketchMusic::Idea^ idea)
		{
			Pos = pos; Layer = layer; Content = idea;
		}

		Windows::Data::Json::IJsonValue^ Serialize();
		static PositionedIdea^ Deserialize(Windows::Data::Json::JsonObject^ json);
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class VerbosePosIdeaCategoryToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			if (value == nullptr) return nullptr;
			PositionedIdea^ idea = (PositionedIdea^)value;
			if (idea->Content == nullptr)
				return "�����";

			IdeaCategoryEnum type = idea->Content->Category;
			//int res = static_cast<int>(type);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "������";
			case IdeaCategoryEnum::chordProgression: return "������������������ ��������";
			case IdeaCategoryEnum::combo: return "��������������� ����";
			case IdeaCategoryEnum::dynamic: return "��������";
			case IdeaCategoryEnum::instrument: return "����������";
			case IdeaCategoryEnum::lyrics: return "�����/�����";
			case IdeaCategoryEnum::melodicSequence: return "������������ ������������������";
			case IdeaCategoryEnum::melody: return "�������";
			case IdeaCategoryEnum::name: return "��������";
			case IdeaCategoryEnum::rhythm: return "����";
			case IdeaCategoryEnum::shape: return "�����";
			case IdeaCategoryEnum::tempo: return "����";
			case IdeaCategoryEnum::none:
			default: return "����������� ���";
			}
			return "";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;
			int res = std::wcstol(dynamic_cast<String^>(value)->Data(), NULL, 10);
			return res;
		}

		VerbosePosIdeaCategoryToStrConverter() {}
	};
}