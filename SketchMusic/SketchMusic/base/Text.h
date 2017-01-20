#pragma once

#include "../SketchMusic.h"
#include "../player/Instrument.h"
#include <map>
#include "Cursor.h"
#include <string>
#include "collection.h"

using namespace SketchMusic;
using namespace Windows::Foundation::Collections;

ref class Text;

/**
 * Текст объединяет в себе отедельные символы-ноты и упрощает работу с ними.
 * Не уверен, что для него нужен отдельный класс, возможно можно обойтись просто map,
 * но я решил ввести класс обёртку, Мало ли какие операции могут пригодиться,
 * особенно когда введу класс слов. Надо будет упростить работу с группами нот.
 * Может даже стоит сделать из этого текста карту слов, а не нот.
 * Но на практике я убедился, что это не совсем удобно и я всё больше считаю, что
 * слова должны существовать для внешнего представления.
 *
 */
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
[Windows::UI::Xaml::Data::BindableAttribute]
public ref class SketchMusic::Text sealed : IIterable<PositionedSymbol^>
{
internal:
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
	
	property SketchMusic::Instrument^ instrument;	// инструмент, который связывается с данной дорожкой
	property SketchMusic::Cursor^ curPosition;

	void addSymbol(PositionedSymbol^ sym) { _t.insert(std::make_pair(sym->_pos, sym->_sym)); }
	void addSymbol(Cursor^ cur, ISymbol^ sym) { _t.insert(std::make_pair(cur, sym)); }
	void addOrReplaceSymbol(PositionedSymbol^ sym);

	ISymbol^ getSymbol(Cursor^ cur) { return _t.find(cur)->second; }
	void deleteSymbol(Cursor^ cur, ISymbol^ sym);
	void deleteSymbols(Cursor^ begin, Cursor^ end);
	void deleteNLine(Cursor^ cur);
	
	void moveSymbol(PositionedSymbol^ psym, SketchMusic::Cursor^ newpos);
	
	// Работа с долями - вставка, удаление
	void addBeat(Cursor^ position, Cursor^ offset);
	void deleteBeat(Cursor^ position, Cursor^ offset);
	//void MoveSymbols(Cursor^ leftBound, Cursor^ rightBound, Cursor^ to);

	unsigned int getSize() { return _t.size(); }
	IVector<PositionedSymbol^>^ getText();
	IVector<PositionedSymbol^>^ GetSymbols(Cursor^ from, Cursor^ to);
	
	// получение нот в конкретной точке и сбоку от неё - для последовательного редактирования
	IVector<PositionedSymbol^>^ getNotesAt(Cursor^ pos, SymbolType type);
	IVector<PositionedSymbol^>^ getNotesAtExcluding(Cursor^ pos, SymbolType type);
	Cursor^ getPosAtLeft(Cursor^ pos);
	Cursor^ getPosAtRight(Cursor^ pos);
	
	static Text^ deserialize(Platform::String^ str);
	static Text^ deserialize(Windows::Data::Json::JsonObject^ json);
	Windows::Data::Json::IJsonValue^ serialize();

	virtual IIterator<PositionedSymbol^>^ First();
	
	// TODO : функции вставки одного текста в другой, объединение, "вырезание" и так далее
	// Потребуются, когда можно будет идеи объединять друг с другом

	
};

public ref class SketchMusic::TextIterator sealed : IIterator<PositionedSymbol^>
{
internal:
	std::multimap<Cursor^, ISymbol^>::iterator m_iter;
	std::multimap<Cursor^, ISymbol^>::iterator end_iter;

public:
	TextIterator(Text^ txt) { m_iter = txt->_t.begin(); end_iter = txt->_t.end(); }

	// Унаследовано через IIterator
	virtual property SketchMusic::PositionedSymbol ^ Current
	{
		PositionedSymbol^ get() { return ref new PositionedSymbol(m_iter->first,m_iter->second); }
	}
	virtual property bool HasCurrent
	{
		bool get() { return m_iter == end_iter; }
	}
	virtual bool MoveNext()
	{
		m_iter++;
		if (m_iter == end_iter) return false;
		else return true;
	}
	virtual unsigned int GetMany(Platform::WriteOnlyArray<SketchMusic::PositionedSymbol ^, 1U> ^items) { return 0; }
};
