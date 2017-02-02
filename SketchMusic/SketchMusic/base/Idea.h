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
		// конструкторы

		//Idea(Platform::String^ _name) { Name = _name; }
		Idea();
		Idea(String^ _name, IdeaCategoryEnum _cat);
		Idea(int _hash, String^ _name, IdeaCategoryEnum _cat, String^ _desc, int _parent, byte _rating,
			CompositionData^ _content, String^ tags, String^ projects, long long _created, long long _modified); // для загрузки
		Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
			Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified);	// чтобы не десериализовывать контент

		CompositionData^ GetContent();

		// свойства

		property int Hash;						// уникальный для каждой идеи
		property IdeaCategoryEnum Category;		// к какой категории относится идея
		property Platform::String^ Name;		// имя для идеи
		property Platform::String^ Description;	// описание идеи
		//SketchMusic::Idea^	Parent;			// ссылка на родительскую идею.
		property int Parent;					// хэш родительской идеи
		property byte Rating;					// оценка, данная данной идее

			// ссылки на объекты не продержатся дальше первого же удаления-восстановления объектов из бд (мы же собираемся их хранить?)

			// Может, это и не актуально, но мне кажется, что будет весьма полезно иметь связи с исходной идеей.
			// Также можно будет составлять список "дочерних" идей
			// TODO : "множественное наследование" ? - идея может быть дочерней сразу для нескольких сразу.
			// однако, сейчас (2016.12.17) предполагается, что будет активно использоваться функция "Развить идею",
			// которая будет опираться прежде всего на некий исходный мотив. На другие мотивы в принципе тоже можно ссылаться,
			// об том стоит подумать

		property CompositionData^ Content;	// собственно, в чём идея заключается
		property Platform::String^ SerializedContent;	// даже если мы не воспользуемся идеей, 
			// для отображения объект создавать всё равно придётся. Чтобы не тратить время, содержимое будем держать
			// в сериализованном виде, пока не потребуется открыть.
		property Platform::String^ Tags;		// список тегов, по которым можно будет искать идею
		//Windows::Foundation::Collections::IVector<String^>^
		property Platform::String^ Projects;	// в каких проектах идея задействована
		property long long CreationTime;		// время и дата создания идеи. Пригодится для сортировки и фильтрации
		property long long ModifiedTime;

		// Унаследовано через INotifyPropertyChanged
		//virtual event PropertyChangedEventHandler ^ PropertyChanged;
		//void RaisePropertyChanged() { PropertyChanged(this, ref new PropertyChangedEventArgs("")); }

		// время последней модификации - нужно?
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
			// TODO : сделать через форматирование
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

	// Класс для размещения идеи в тексте. Аналогично PositionedSymbol имеет точку вставки, но помимо этого содержит так же длину
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class PositionedIdea sealed
	{
	private:
		PositionedIdea() {}
	public:
		// вообще говоря, вряд ли мы будем пользоваться прямо-таки точным расположением и
		// в большинстве случаев скорее всего хватит указания номера доли.

		property SketchMusic::Cursor^ Pos;		// исходное положение
		property SketchMusic::Cursor^ Length;	// длина. Праввая граница рассчитывается как Pos + Length
		property int Layer;						// слой, на котором располагается идея

		property SketchMusic::Idea^ Content;		// собствено идея

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
				return "Пусто";

			IdeaCategoryEnum type = idea->Content->Category;
			//int res = static_cast<int>(type);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "Аккорд";
			case IdeaCategoryEnum::chordProgression: return "Последовательность аккордов";
			case IdeaCategoryEnum::combo: return "Комбинированная идея";
			case IdeaCategoryEnum::dynamic: return "Динамика";
			case IdeaCategoryEnum::instrument: return "Инструмент";
			case IdeaCategoryEnum::lyrics: return "Текст/стихи";
			case IdeaCategoryEnum::melodicSequence: return "Мелодическая последовательность";
			case IdeaCategoryEnum::melody: return "Мелодия";
			case IdeaCategoryEnum::name: return "Название";
			case IdeaCategoryEnum::rhythm: return "Ритм";
			case IdeaCategoryEnum::shape: return "Форма";
			case IdeaCategoryEnum::tempo: return "Темп";
			case IdeaCategoryEnum::none:
			default: return "Неизвестный тип";
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