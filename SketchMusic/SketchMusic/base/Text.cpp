#include "pch.h"
#include "Text.h"

using namespace SketchMusic;
using Windows::Foundation::Collections::IVector;
using Platform::Collections::Vector;
using namespace Windows::Data::Json;

typedef std::multimap < Cursor^, ISymbol^ >  psymMap;
typedef psymMap::iterator psymIter;

Text ^ SketchMusic::Text::deserializeFromString(Platform::String ^ serialized)
{
	Text^ text = ref new Text();

	// инструмент

	// массив с нотами

	return text;
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
	// инструмент в начале
	// потом список со всеми нотами
	String^ result = this->instrument->_name + ";";
	for (std::pair<Cursor^, ISymbol^> sym : this->_t)
	{
		result += sym.first->ToString() + "=" + sym.second->ToString() + ";";
	}
	return result;
}

IJsonValue^ SketchMusic::Text::serialize()
{
	JsonObject^ json = ref new JsonObject();
	json->Insert("instr", JsonValue::CreateStringValue(instrument->_name));	
		// TODO : ключи дл€ поиска значений должны Ќ≈ быть встроенными! ¬ынести отдельно
	JsonArray^ jsonNotes = ref new JsonArray;
	for (auto sym : this->_t)
	{
		JsonObject^ jsym = ref new JsonObject;
		jsym->Insert("p1", JsonValue::CreateNumberValue(sym.first->getBeat()));
		jsym->Insert("p2", JsonValue::CreateNumberValue(sym.first->getTick()));
		jsym->Insert("t", JsonValue::CreateNumberValue(static_cast<int>(sym.second->GetSymType())));
		auto note = dynamic_cast<INote^>(sym.second);
		if (note)
		{
			jsym->Insert("v",JsonValue::CreateNumberValue(note->_val));
		}
		jsonNotes->Append(jsym);
	}
	json->Insert("notes", jsonNotes);
	return json;
}

void SketchMusic::Text::deserialize(Platform::String^ str)
{
	JsonObject^ json = ref new JsonObject();
	if (JsonObject::TryParse(str, &json))
	{
		//use the JsonObject json
		this->instrument = ref new Instrument(json->GetNamedString("instr"));
		auto jArr = json->GetNamedArray("notes");
		// проходимс€ по массиву и создаЄм нотки
		for (auto i : jArr)
		{
			JsonObject^ jEl = i->GetObject();
			if (jEl)
			{
				int pos_tick = static_cast<int>(jEl->GetNamedNumber("p1"));
				int pos_beat = static_cast<int>(jEl->GetNamedNumber("p2"));
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
		// если такой символ имеетс€
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
	// - начина€ с position сдвигаем все последующие ноты на offset
	// - чо-то как-то не очень. ≈сть вариант разбивать все символы на строки - 
	// тогда сдвигать надо будет только символы в рамках одной строки
}

void SketchMusic::Text::deleteBeat(Cursor^ position, Cursor^ offset)
{
	// TODO
	// - начина€ с position сдвигаем все последующие ноты на offset
	// - выпадающие ноты удал€ем
}