#include "pch.h"
#include "Text.h"

using namespace SketchMusic;
using Windows::Foundation::Collections::IVector;
using Platform::Collections::Vector;
using namespace Windows::Data::Json;

typedef std::multimap < Cursor^, ISymbol^ >  psymMap;
typedef psymMap::iterator psymIter;
namespace t = SketchMusic::SerializationTokens;

SketchMusic::Text::Text()
{
	this->curPosition = ref new Cursor;
}

SketchMusic::Text::Text(Instrument^ instrument)
{
	this->curPosition = ref new Cursor;
	this->instrument = instrument;
}

SketchMusic::Text::Text(String^ instrumentName)
{
	this->curPosition = ref new Cursor;
	this->instrument = ref new Instrument(instrumentName);
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

IJsonValue^ SketchMusic::Text::serialize()
{
	JsonObject^ json = ref new JsonObject();
	json->Insert(t::INSTR, JsonValue::CreateStringValue(instrument->_name));	
		// TODO : ключи для поиска значений должны НЕ быть встроенными! Вынести отдельно
	JsonArray^ jsonNotes = ref new JsonArray;
	for (auto sym : this->_t)
	{
		JsonObject^ jsym = ref new JsonObject;
		jsym->Insert(t::BEAT, JsonValue::CreateNumberValue(sym.first->getBeat()));
		jsym->Insert(t::TICK, JsonValue::CreateNumberValue(sym.first->getTick()));
		jsym->Insert(t::SYMBOL_TYPE, JsonValue::CreateNumberValue(static_cast<int>(sym.second->GetSymType())));
		auto note = dynamic_cast<INote^>(sym.second);
		if (note)
		{
			jsym->Insert(t::NOTE_VALUE,JsonValue::CreateNumberValue(note->_val));
			if (note->_velocity) { jsym->Insert(t::NOTE_VELOCITY, JsonValue::CreateNumberValue(note->_velocity)); }
			if (note->_velocity) { jsym->Insert(t::NOTE_VOICE, JsonValue::CreateNumberValue(note->_voice)); }
		}
		jsonNotes->Append(jsym);
	}
	json->Insert(t::NOTES_ARRAY, jsonNotes);
	return json;
}

void SketchMusic::Text::deserialize(Platform::String^ str)
{
	JsonObject^ json = ref new JsonObject();
	if (JsonObject::TryParse(str, &json))
	{
		//use the JsonObject json
		this->instrument = ref new Instrument(json->GetNamedString(t::INSTR));
		auto jArr = json->GetNamedArray(t::NOTES_ARRAY);
		// проходимся по массиву и создаём нотки
		for (auto i : jArr)
		{
			JsonObject^ jEl = i->GetObject();
			if (jEl)
			{
				int pos_tick = static_cast<int>(jEl->GetNamedNumber(t::BEAT));
				int pos_beat = static_cast<int>(jEl->GetNamedNumber(t::TICK));
				auto pos = ref new Cursor(pos_tick, pos_beat);

				auto sym = ISymbolFactory::Deserialize(jEl);

				_t.insert(std::make_pair(pos, sym));
			}
		}
	}
	
	return;
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