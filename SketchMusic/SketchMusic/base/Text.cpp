#include "pch.h"
#include "Text.h"

using namespace SketchMusic;
using Windows::Foundation::Collections::IVector;
using Platform::Collections::Vector;

typedef std::multimap < Cursor^, ISymbol^ >  psymMap;
typedef psymMap::iterator psymIter;

Text ^ SketchMusic::Text::deserializeFromString(Platform::String ^ serialized)
{
	return nullptr;
}

SketchMusic::Text::Text()
{
	this->curPosition = ref new Cursor;
	//this->Ops = "koko";
}

SketchMusic::Text::Text(Instrument^ instrument)
{
	this->curPosition = ref new Cursor;
	this->instrument = instrument;
	//this->Ops = "koko";
}

SketchMusic::Text::Text(String^ instrumentName)
{
	this->curPosition = ref new Cursor;
	this->instrument = ref new Instrument(instrumentName);
	//this->Ops = "koko";
}

void SketchMusic::Text::deleteSymbol(Cursor^ cur, ISymbol^ sym)
{
	// TODO 
	/*for (std::pair<SketchMusic::Cursor^, SketchMusic::ISymbol^> sym : this->_t)
	{

	}*/
	//this->_t[cur] = sym;
}

void SketchMusic::Text::deleteSymbols(Cursor^ begin, Cursor^ end)
{
	if (begin->EQ(end))
		return;

	bool inverse = end->LT(begin);
	SketchMusic::Cursor^ left = (!inverse) ? begin : end;
	SketchMusic::Cursor^ right = (!inverse) ? end : begin;

	auto leftIter = _t.lower_bound(left);
	if (leftIter == _t.end())
		return;

	auto rightIter = _t.lower_bound(right);

	//for (std::pair<SketchMusic::Cursor^, SketchMusic::ISymbol^> sym : this->_t)
	/*for (auto iter = _t.begin(); iter != _t.end(); iter++)
	{
		if (((*iter).first->EQ(left)) || (((*iter).first->GT(left)) && ((*iter).first->LT(right))))
		{
			if ((leftIter. iter) leftIter = iter;
		}
	}*/

	_t.erase(leftIter,rightIter);
}

IVector<PositionedSymbol^>^ SketchMusic::Text::getText()
{
	Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;
	
	for (std::pair<Cursor^, ISymbol^> sym : this->_t)
	{
		vect->Append(ref new PositionedSymbol(sym.first,sym.second));
	}

	return vect;
}

Platform::String ^ SketchMusic::Text::serializeToString()
{
	return "";
}

void SketchMusic::Text::moveSymbol(SketchMusic::PositionedSymbol^ psym, SketchMusic::Cursor^ newpos)
{
	psymIter target = _t.end();
	for (auto sym = _t.lower_bound(psym->_pos); sym->first->EQ(psym->_pos); sym++)
	{
		// если такой символ имеется
		if (sym->second == psym->_sym)
		{
			target = sym;
			break;
		}
	}

	// перемещаем его в новое положение
	if (target != _t.end())
	{
		_t.erase(target);
		_t.insert(std::make_pair(newpos, psym->_sym));
	}
}

void SketchMusic::Text::addBeat(Cursor^ position, Cursor^ offset)
{
	// TODO
	// - начиная с position сдвигаем все последующие ноты на offset
	// - чо-то как-то не очень. Есть вариант разбивать все символы на строки - 
	// тогда сдвигать надо будет только символы в рамках одной строки
}

void SketchMusic::Text::deleteBeat(Cursor^ position, Cursor^ offset)
{
	// TODO
	// - начиная с position сдвигаем все последующие ноты на offset
	// - выпадающие ноты удаляем
}