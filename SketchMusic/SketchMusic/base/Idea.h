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
		// конструкторы

		//Idea(Platform::String^ _name) { Name = _name; }
		Idea();
		Idea(String^ _name, IdeaCategoryEnum _cat);
		Idea(int _hash, String^ _name, IdeaCategoryEnum _cat, String^ _desc, int _parent, byte _rating,
			 SketchMusic::Text^ _content, String^ tags, String^ projects, long long _created, long long _modified); // для загрузки
		Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
			Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified);	// чтобы не десериализовывать контент

		SketchMusic::Text^ GetContent();

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

		property SketchMusic::Text^ Content;	// собственно, в чём идея заключается
		property Platform::String^ SerializedContent;	// даже если мы не воспользуемся идеей, 
			// для отображения объект создавать всё равно придётся. Чтобы не тратить время, содержимое будем держать
			// в сериализованном виде, пока не потребуется открыть.
		property Platform::String^ Tags;		// список тегов, по которым можно будет искать идею
		//Windows::Foundation::Collections::IVector<String^>^
		property Platform::String^ Projects;	// в каких проектах идея задействована
		property long long CreationTime;		// время и дата создания идеи. Пригодится для сортировки и фильтрации
		property long long ModifiedTime;		// время последней модификации - нужно?
	};
}