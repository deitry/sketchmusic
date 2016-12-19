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
		// конструкторы

		Idea() {}	// удалить его вообще?
		//Idea(Platform::String^ _name) { Name = _name; }
		Idea(String^ _name, IdeaCategoryEnum _cat);
		
		// свойства

		property IdeaCategoryEnum Category;		// к какой категории относится идея
		property Platform::String^ Name;		// имя для идеи
		property Platform::String^ hash;		// нужен ? уникальный для каждой идеи
		property Platform::String^ Description;	// описание идеи
		//SketchMusic::Idea^	Parent;			// ссылка на родительскую идею.
		property Platform::String^	Parent;		// ссылка на родительскую идею в виде хэша
		property byte rating;					// оценка, данная данной идее
		
			// ссылки на объекты не продержатся дальше первого же удаления-восстановления объектов из бд (мы же собираемся их хранить?)

			// Может, это и не актуально, но мне кажется, что будет весьма полезно иметь связи с исходной идеей.
			// Также можно будет составлять список "дочерних" идей
			// TODO : "множественное наследование" ? - идея может быть дочерней сразу для нескольких сразу.
			// однако, сейчас (2016.12.17) предполагается, что будет активно использоваться функция "Развить идею",
			// которая будет опираться прежде всего на некий исходный мотив. На другие мотивы в принципе тоже можно ссылаться,
			// об том стоит подумать

		
	internal:
	
		SketchMusic::Text^ Content;				// собственно, в чём идея заключается
		Windows::Foundation::Collections::IVector<String^>^ tags;		// список тегов, по которым можно будет искать идею
		Windows::Foundation::Collections::IVector<String^>^ projects;	// в каких проектах идея задействована
		DateTime creationTime;	// время и дата создания идеи. Пригодится для сортировки и фильтрации
		DateTime modifiedTime;	// время последней модификации - нужно?
	};
}