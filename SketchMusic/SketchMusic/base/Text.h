#pragma once

#include "../SketchMusic.h"
#include "../player/Instrument.h"
#include <map>
#include "Cursor.h"
#include <string>
#include "collection.h"

using namespace SketchMusic;
using namespace Windows::Foundation::Collections;

/**
 * Текст объединяет в себе отедельные символы-ноты и упрощает работу с ними.
 * Не уверен, что для него нужен отдельный класс, возможно можно обойтись просто map,
 * ноя решил ввести класс обёртку, Мало ди какие операции могут пригодиться,
 * особенно когда введу еласс слов. Надо будет упростить работу с группами нот.
 * Может даже стоит сделать из этого текста карту слов, а не нот.
 * Но на практике я убедился, что это не совсем удобно и я всё больше считаю, что
 * слова должны существовать для внешнего представления.
 *
 */
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
[Windows::UI::Xaml::Data::BindableAttribute]
public ref class SketchMusic::Text sealed
{
private:
	std::multimap<Cursor^, ISymbol^> _t;
		// чтобы можно было вставлять несколько символов с одним положением.
		// При этом нужно будет сделать обёртку для адд, чтобы ноты объединять в аккорды,
		// а системные символы вставлять как обычно или заменять один на другой.
		// todo : надо ли менять на указатель на объект или лучше оставить вот так, классом? Не знаю.
	
		// todo : нужно подумать насчёт улучшения подхода. При добавлении новых долей придётся
		// проходиться по всему тексту и смещать каждую ноту, как-то это тупо

		// для каждого элемента хранить не абсолютное положение, а относительное - относительно текущего элемента?
		// абсолютное - можно как буферизованное значение, вычисляемое для последнего обращения к одному из элементов...
public:
	Text();
	Text(Windows::Data::Json::JsonObject^ json) {}
	Text(Instrument^ instrument);
	//Text(String^ instrumentName);
		// TODO : так ли важен используемый инструмент для определения текста?
		// Если использовать этот класс для определения содержимого произвольной идеи,
		// то инструмент может сам по себе быть идеей, а может не играть никакой роли вообще
		// ! Однако инструмент требуется в любом случае, если мы захотим прослушать то, что в тексте написано.
		// Соответственно без указания инструмента для озвучивания остаётся только использовать некий "дефолтный" инструмент

	static SketchMusic::Text^ deserialize(Platform::String^ str);

	property SketchMusic::Instrument^ instrument;	// инструмент, который связывается с данной дорожкой
	property SketchMusic::Cursor^ curPosition;

	void addSymbol(PositionedSymbol^ sym) { _t.insert(std::make_pair(sym->_pos, sym->_sym)); }
	void addSymbol(Cursor^ cur, ISymbol^ sym) { _t.insert(std::make_pair(cur, sym)); }
	ISymbol^ getSymbol(Cursor^ cur) { return _t.find(cur)->second; }
	void deleteSymbol(Cursor^ cur, ISymbol^ sym);
	void deleteSymbols(Cursor^ begin, Cursor^ end);
	
	void moveSymbol(PositionedSymbol^ psym, SketchMusic::Cursor^ newpos);
	
	// Работа с долями - вставка, удаление
	void addBeat(Cursor^ position, Cursor^ offset);
	void deleteBeat(Cursor^ position, Cursor^ offset);

	IVector<PositionedSymbol^>^ getText();
	//Platform::Collections::Vector<PositionedSymbol^>^ getText();
	
	Windows::Data::Json::IJsonValue^ serialize();
	
	// TODO : функции вставки одного текста в другой, объединение, "вырезание" и так далее
	// Потребуются, когда можно будет идеи объединять друг с другом
};

