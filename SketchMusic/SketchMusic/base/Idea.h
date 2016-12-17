#pragma once

#include "../SketchMusic.h"

using namespace SketchMusic;

public ref class SketchMusic::Idea sealed
{
	property Platform::String^ Name;		// им€ дл€ идеи
	property Platform::String^ Description;	// описание идеи
	property IdeaCategoryEnum Category;		// к какой категории относитс€ иде€
	SketchMusic::Idea^	Parent;				// ссылка на родительскую идею.
		// ћожет, это и не актуально, но мне кажетс€, что будет весьма полезно иметь св€зи с исходной идеей.
		// “акже можно будет составл€ть список "дочерних" идей
		// TODO : "множественное наследование" ? - иде€ может быть дочерней сразу дл€ нескольких сразу.
		// однако, сейчас (2016.12.17) предполагаетс€, что будет активно использоватьс€ функци€ "–азвить идею",
		// котора€ будет опиратьс€ прежде всего на некий исходный мотив. Ќа другие мотивы в принципе тоже можно ссылатьс€,
		// об том стоит подумать
	SketchMusic::Text^ Content;				// собственно, в чЄм иде€ заключаетс€
	Windows::Foundation::Collections::IVector<String^>^ tags;		// список тегов, по которым можно будет искать идею
	Windows::Foundation::Collections::IVector<String^>^ projects;	// в каких проектах иде€ задействована
};