#include "pch.h"
#include "Text.h"
#include "Cursor.h"

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

//SketchMusic::Text::Text(String^ instrumentName)
//{
//	this->curPosition = ref new Cursor;
//	this->instrument = ref new Instrument(instrumentName);
//}

void SketchMusic::Text::addOrReplaceSymbol(PositionedSymbol ^ sym)
{
	// поискать аналогичный символ в этом месте
	auto lookup = _t.lower_bound(sym->_pos);
	auto max = _t.upper_bound(sym->_pos);
	if (lookup == max)
	{
		// если нету - вставляем новый символ
		_t.insert(std::make_pair(sym->_pos,sym->_sym));
	}
	else
	{
		// если нашли - изменяем содержимое
		for (;lookup != max; lookup++)
		{
			if (lookup->second->GetSymType() == sym->_sym->GetSymType())
			{
				lookup->second = sym->_sym;
			}
		}
	}
}

void SketchMusic::Text::deleteSymbol(Cursor^ cur, ISymbol^ sym)
{
	auto lookup = _t.lower_bound(cur);
	auto max = _t.upper_bound(cur);
	for (;lookup != max; lookup++)
	{
		auto type = lookup->second->GetSymType();
		if (type == sym->GetSymType())
		{
			// проверка на то, что это действительно тот символ - помимо типа проверяем на значение
			switch (type)
			{
			case SymbolType::NOTE:
			case SymbolType::RNOTE:
			case SymbolType::GNOTE:
			{
				INote^ note = dynamic_cast<INote^>(lookup->second);
				INote^ note2 = dynamic_cast<INote^>(sym);
				if (note->_val == note2->_val)
				{
					_t.erase(lookup);
					return;
				}
				break;
			}
			default:
				// по умолчанию на тип не смотрим, полагая, что там только один такой символ
				_t.erase(lookup);
				return;
			}
		}
	}
}

// Удаляем разрыв строки из текста. Если удалили - возвращаем true, если такого символа нету - false
bool SketchMusic::Text::deleteNLine(Cursor^ cur)
{
	// проходимся по всем нотам в этом месте
	// если есть SNewLine - удаляем - только один!
	auto psym = _t.find(cur);
	if (psym == _t.end())
		return false;

	auto left = _t.lower_bound(cur);
	auto right = _t.upper_bound(cur);
	for (; left != right; left++)
	{
		if (left->second->GetSymType() == SketchMusic::SymbolType::NLINE)
		{
			_t.erase(left);
			return true;
		}
	}
	return false;
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

IVector<PositionedSymbol^>^ SketchMusic::Text::GetSymbols(Cursor ^ from, Cursor ^ to)
{
	Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;

	if (from== nullptr && to == nullptr) return vect;

	auto left = from ? _t.lower_bound(from) : _t.begin();
	if (left == _t.end()) return vect;

	auto right = ((to) ? ((from->EQ(to)) ? _t.upper_bound(from) : _t.lower_bound(to)) : _t.end());
	if (left == right) return vect;

	//for (std::pair<Cursor^, ISymbol^> sym : this->_t)
	for (; left != right; left++)
	{
		vect->Append(ref new PositionedSymbol(left->first, left->second));
	}

	return vect;
}

// Получаем ноты в данной позиции, имеющие тип type (или все, если type == unknown)
IVector<PositionedSymbol^>^ SketchMusic::Text::getNotesAt(Cursor ^ pos, SymbolType type)
{
	if (pos == nullptr) return nullptr;
	Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;
	
	for (auto sym = _t.lower_bound(pos); sym != _t.upper_bound(pos); sym++)
	{
		if (type == SymbolType::unknown || type == sym->second->GetSymType())
			vect->Append(ref new PositionedSymbol(sym->first, sym->second));
	}

	return vect;
}

// Получаем ноты в данной позиции, исключая те, которые имеют тип type (или все, если type == unknown)
IVector<PositionedSymbol^>^ SketchMusic::Text::getNotesAtExcluding(Cursor ^ pos, SymbolType type)
{
	if (pos == nullptr) return nullptr;
	Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;

	for (auto sym = _t.lower_bound(pos); sym != _t.upper_bound(pos); sym++)
	{
		if (type == SymbolType::unknown || type != sym->second->GetSymType())
			vect->Append(ref new PositionedSymbol(sym->first, sym->second));
	}

	return vect;
}

Cursor^ SketchMusic::Text::getPosAtLeft(Cursor ^ pos)
{
	auto curIter = _t.lower_bound(pos);
	// проверка на то, что мы не в самом начале
	if (curIter == _t.begin()) return nullptr;

	auto prevPos = (--curIter)->first;
	//Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;
	//for (auto sym = _t.lower_bound(prevPos); sym != _t.upper_bound(prevPos); sym++)
	//{
	//	vect->Append(ref new PositionedSymbol(sym->first, sym->second));
	//}

	return prevPos;
}

Cursor^ SketchMusic::Text::getPosAtRight(Cursor ^ pos)
{
	auto nextIter = _t.upper_bound(pos);
	// проверка на то, что мы не в самом конце
	if (nextIter == _t.end()) return nullptr;
	
	auto nextPos = nextIter->first;
	//Vector<PositionedSymbol^>^ vect = ref new Vector<PositionedSymbol^>;
	//
	//for (auto sym = _t.lower_bound(nextPos); sym != _t.upper_bound(nextPos); sym++)
	//{
	//	if (type == SymbolType::unknown || type == sym->second->GetSymType())
	//		vect->Append(ref new PositionedSymbol(sym->first, sym->second));
	//}

	return nextPos;
}

IJsonValue^ SketchMusic::Text::serialize()
{
	JsonObject^ json = ref new JsonObject();
	if (instrument)
	{
		json->Insert(t::INSTR, JsonValue::CreateStringValue(instrument->_name));

		if (instrument->preset && instrument->preset != "")
		{
			json->Insert(t::INSTR_PRESET, JsonValue::CreateStringValue(instrument->preset)); 
		}
	}
	JsonArray^ jsonNotes = ref new JsonArray;
	for (auto sym : this->_t)
	{
		// не вызываем сериализацию для каждого элемента в отдельности, чтобы немного сэкономить на объектах.
		JsonObject^ jsym = ref new JsonObject;
		jsym->Insert(t::BEAT, JsonValue::CreateNumberValue(sym.first->Beat));
		jsym->Insert(t::TICK, JsonValue::CreateNumberValue(sym.first->Tick));
		jsym->Insert(t::SYMBOL_TYPE, JsonValue::CreateNumberValue(static_cast<int>(sym.second->GetSymType())));
		auto tempo = dynamic_cast<STempo^>(sym.second);
		if (tempo)
		{
			jsym->Insert(t::NOTE_VALUE, JsonValue::CreateNumberValue(tempo->value));
		}
		auto part = dynamic_cast<SNewPart^>(sym.second);
		if (part)
		{
			jsym->Insert(t::PART_NAME, JsonValue::CreateStringValue(part->Name));
			jsym->Insert(t::PART_CAT, JsonValue::CreateStringValue(part->category));
			jsym->Insert(t::PART_DYN, JsonValue::CreateNumberValue((int)part->dynamic));
		}
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

IIterator<PositionedSymbol^>^ SketchMusic::Text::First()
{
	TextIterator^ iter = ref new TextIterator(this);
	return iter;
}

SketchMusic::Text^ SketchMusic::Text::deserialize(Platform::String^ str)
{
	JsonObject^ json = ref new JsonObject();
	if (JsonObject::TryParse(str, &json))
	{
		return SketchMusic::Text::deserialize(json);
	}
	else return nullptr;
}

SketchMusic::Text ^ SketchMusic::Text::deserialize(Windows::Data::Json::JsonObject ^ json)
{
	SketchMusic::Text^ text = ref new SketchMusic::Text;
	text->instrument = ref new Instrument(json->GetNamedString(t::INSTR), 
										  json->GetNamedString(t::INSTR_PRESET,nullptr));

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

			text->_t.insert(std::make_pair(pos, sym));
		}
	}
	return text;
}

void SketchMusic::Text::moveSymbol(SketchMusic::PositionedSymbol^ psym, SketchMusic::Cursor^ newpos)
{
	psymIter target = _t.end();
	for (auto sym = _t.lower_bound(psym->_pos); sym != _t.upper_bound(psym->_pos); sym++)
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
