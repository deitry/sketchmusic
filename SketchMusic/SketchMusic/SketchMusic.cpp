// Class1.cpp
#include "pch.h"
#include "SketchMusic.h"
#include "base/Text.h"

using namespace SketchMusic;
using namespace Platform;
using namespace Windows::Data::Json;
using SketchMusic::SymbolType;
namespace t = SketchMusic::SerializationTokens;

Platform::String^ SketchMusic::SNote::valToString(int val)
{
	return "0";
}

Platform::String^ SketchMusic::SRNote::valToString(int val)
{
	return "0";
}

Platform::String^ SketchMusic::SGNote::valToString(int val)
{
	return "0";
}

ISymbol ^ SketchMusic::ISymbolFactory::Deserialize(JsonObject^ obj)
{
	//auto obj = dynamic_cast<JsonObject^>(json);
	ISymbol^ result;
	if (obj)
	{
		int val = 0;
		int voi = 0;
		int vel = 0;

		auto jval = obj->GetNamedValue(t::SYMBOL_TYPE);
		if (jval)
		{
			const wchar_t* begin = jval->Stringify()->Data();
			SymbolType sym_type = static_cast<SymbolType>(std::wcstol(begin, nullptr, 10));
			switch (sym_type)
			{
			case SymbolType::NOTE:
			{
				val = obj->GetNamedNumber(t::NOTE_VALUE);
				vel = obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = obj->GetNamedNumber(t::NOTE_VOICE,0);
				return ref new SNote(val);
			}
			case SymbolType::RNOTE:
			{
				val = obj->GetNamedNumber(t::NOTE_VALUE);
				vel = obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = obj->GetNamedNumber(t::NOTE_VOICE,0);
				return ref new SRNote(val);
			}
			case SymbolType::GNOTE:
			{
				val = obj->GetNamedNumber(t::NOTE_VALUE);
				vel = obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = obj->GetNamedNumber(t::NOTE_VOICE,0);
				return ref new SGNote(val);
			}
			case SymbolType::NPART:
			{
				auto name = obj->GetNamedString(t::PART_NAME, "");
				auto cat = obj->GetNamedString(t::PART_CAT, "");
				auto dyn = (DynamicCategory)((int)obj->GetNamedNumber(t::PART_DYN, 0));

				return ref new SNewPart(name, cat, dyn);
			}
			case SymbolType::TEMPO:
			{
				vel = obj->GetNamedNumber(t::NOTE_VALUE, 120.);
				return ref new STempo(vel);
			}
			case SymbolType::END:
			{
				return ref new SNoteEnd();
			}
			case SymbolType::NLINE:
			{
				return ref new SNewLine();
			}
			case SymbolType::SPACE:
			{
				return ref new SSpace();
			}
			case SymbolType::ACCENT:
			{
				return ref new SSpace();
			}
			case SymbolType::CHORD:
			case SymbolType::STRING:
			default: return nullptr;
			}
		}
	}
	return nullptr;
}

ISymbol ^ SketchMusic::ISymbolFactory::CreateSymbol(SketchMusic::View::KeyType type, Object^ val)
{
	switch (type)
	{
	case SketchMusic::View::KeyType::note:
	{
		int ival = (int)val;
		return ref new SNote(ival);
	}
	case SketchMusic::View::KeyType::relativeNote:
	{
		int ival = (int)val;
		return ref new SRNote(ival);
	}
	case SketchMusic::View::KeyType::genericNote:
	{
		int ival = (int)val;
		return ref new SGNote(ival);
	}
	case SketchMusic::View::KeyType::newPart:
	{
		auto str = (Platform::String^)val;
		return ref new SNewPart(str);
	}
	case SketchMusic::View::KeyType::tempo:
	{
		int ival = 120;
		if (val) ival = (int)val;
		return ref new STempo(ival);
	}
	case SketchMusic::View::KeyType::end:
	{
		return ref new SNoteEnd();
	}
	case SketchMusic::View::KeyType::enter:
	{
		return ref new SNewLine();
	}
	case SketchMusic::View::KeyType::space:
	{
		return ref new SSpace();
	}
	case SketchMusic::View::KeyType::accent:
	{
		return ref new SSpace();
	}
	default: return nullptr;
	}
	return nullptr;
}

// проходимся по всем текстам, собираем из них управляющие символы, действующие для всей композиции
// - метроном, обозначения частей
void SketchMusic::CompositionData::HandleControlSymbols()
{
	for (auto&& txt : m_texts)
	{
		auto symbols = txt->getText();
		for (auto psym : symbols)
		{
			switch (psym->_sym->GetSymType())
			{
				// выбираем все "управляющие символы"
			case SymbolType::NPART:
			case SymbolType::TEMPO:
			{
				// удалить из исходного текста, добавить в контрол
				txt->deleteSymbol(psym->_pos, psym->_sym);
				ControlText->addSymbol(psym);
				break;
			}
			default:
				break;
			}
		}
	}
}

IObservableVector<PartDefinition^>^ SketchMusic::CompositionData::getParts()
{
	auto parts = ref new Platform::Collections::Vector<PartDefinition^>;
	PartDefinition^ prev = nullptr;
	int curBpm = BPM;
	std::pair<Cursor^, ISymbol^> prevTempo;
	for (auto&& i : ControlText->_t)
	{
		if (i.second->GetSymType() == SymbolType::TEMPO)
		{
			auto tempo = dynamic_cast<STempo^>(i.second);
			if (tempo)
			{
				int maxBeat = 0;
				if (prevTempo.first)
				{
					if (prevTempo.first->Beat > maxBeat) maxBeat = prevTempo.first->Beat;
				}
				if (prev)
				{
					if (prev->originalPos && prev->originalPos->Beat > maxBeat) maxBeat = prev->originalPos->Beat;
					prev->Time += (i.first->Beat - maxBeat) * 60 / curBpm;
				}

				curBpm = tempo->value;
				prevTempo = i;
			}
		}
		else if (i.second->GetSymType() == SymbolType::NPART)
		{
			auto def = ref new PartDefinition(i.first, dynamic_cast<SNewPart^>(i.second));
			if (prev)
			{
				int maxBeat = prev->originalPos->Beat;
				if (prevTempo.first && prevTempo.first->Beat > maxBeat) maxBeat = prevTempo.first->Beat;

				prev->Length = def->originalPos->Beat - prev->originalPos->Beat;
				prev->Time += (i.first->Beat - maxBeat) * 60 / curBpm;
			}

			parts->Append(def);
			prev = def;
		}
	}

	// исключаем последний PartDefinition - он обозначает конец композиции и для манипуляций не нужен
	if (parts->Size)
		parts->RemoveAt(parts->Size - 1);
	return parts;
}

CompositionData ^ SketchMusic::CompositionData::deserialize(Platform::String^ str)
{
	SketchMusic::CompositionData^ data = nullptr;
	auto json = ref new JsonArray();
	if (JsonArray::TryParse(str, &json))
	{
		data = deserialize(json);
	}
	else
	{
		// не смогли получить массив - считаем, что только один
		auto obj = ref new JsonObject();
		if (JsonObject::TryParse(str, &obj))
		{
			auto text = SketchMusic::Text::deserialize(obj);
			if (text)
			{
				data = ref new SketchMusic::CompositionData;
				if (text->instrument->_name == SerializationTokens::CONTROL_TEXT)
					data->ControlText = text;
				else data->texts->Append(text);
			}
		}
	}
	return data;
}

CompositionData ^ SketchMusic::CompositionData::deserialize(Windows::Data::Json::JsonArray^ json)
{
	SketchMusic::CompositionData^ data = nullptr;
	data = ref new SketchMusic::CompositionData;
	for (auto i : json)
	{
		auto obj = i->GetObject();
		auto text = SketchMusic::Text::deserialize(obj);
		if (text)
		{
			if (text->instrument->_name == SerializationTokens::CONTROL_TEXT)
				data->ControlText = text;
			else data->texts->Append(text);
		}
	}
	return data;
}

Windows::Data::Json::IJsonValue ^ SketchMusic::CompositionData::serialize()
{
	auto json = ref new JsonArray();
	if (ControlText)
		json->Append(ControlText->serialize());

	for (auto&& text : texts)
	{
		if (text->getSize())
		{
			json->Append(static_cast<SketchMusic::Text^>(text)->serialize());
		}
	}
	return json;
}

void SketchMusic::CompositionData::ApplyParts(IObservableVector<PartDefinition^>^ parts)
{
	// проходимся по списку определений частей
	// формируем новый текст, "выдёргивая" из старого текста символы и расставляя их согласно новой структуре

	// проверка на то, что реорганизовывать ничего не надо
	bool ok = true;
	int cur = 0;
	for (auto&& part : parts)
	{
		if ((part->originalPos == nullptr) || (part->originalPos && part->originalPos->Beat != cur))
		{
			ok = false;
			break;
		}
		cur += part->Length;
	}

	if (ok) return;

	// инициализируем "реорганизованные" тексты
	auto reorginised = ref new Platform::Collections::Vector<Text^>;
	for (auto&& oldText : m_texts)
	{
		Text^ newText = ref new Text(oldText->instrument);
		reorginised->Append(newText);
	}
	Text^ newCtrlTxt = ref new Text(ref new Instrument(t::CONTROL_TEXT));

	// применяем перестановки частей
	int curPos = 0;
	for (auto&& part : parts)
	{
		// определяем граничные итераторы части в данном тексте
		if (part->originalPos != nullptr)
		{
			for (int i = 0; i < m_texts->Size + 1; i++)
			{
				Text^ oldText;
				Text^ newText;
				if (i < m_texts->Size)
				{
					oldText = m_texts->GetAt(i);
					newText = reorginised->GetAt(i);
				}
				else
				{
					oldText = ControlText;
					newText = newCtrlTxt;
				}

				auto orig = part->originalPos;
				auto leftBound = oldText->_t.lower_bound(orig);	// откуда начнём "вырезать" текст
				if (leftBound == oldText->_t.end()) continue;

				int rightBeat = leftBound->first->Beat + part->Length;
				int dif = curPos - orig->Beat;
				Cursor^ rightPos = ref new Cursor(rightBeat);
				auto rightBound = oldText->_t.lower_bound(rightPos);	// где закончим

				// переставляем символы из старого текста в новый
				for (auto iter = leftBound; iter != rightBound; iter++)
				{
					auto sym = iter->second;
					if (sym->GetSymType() == SymbolType::NPART)
						continue;	// переставляем всё, кроме символов частей, их вставим вручную согласно новому списку
					
					if (sym->GetSymType() == SymbolType::NLINE)	// для обратной совместимости с уже существующими проектами
						// символы переноса на строку вставляем в управляющий текст, чтобы сделать их общими для всех текстов
						newCtrlTxt->_t.insert(std::make_pair(ref new Cursor(iter->first->Beat + dif, iter->first->Tick), sym));
					else newText->_t.insert(std::make_pair(ref new Cursor(iter->first->Beat + dif, iter->first->Tick), sym));
				}
			}
		}

		// вставляем собственно сам символ части
		part->originalPos = ref new Cursor(curPos);	// обновляем положение части
		newCtrlTxt->_t.insert(std::make_pair(part->originalPos, part->original));
		curPos += part->Length;
	}

	// вставляем символ части в самый конец
	newCtrlTxt->_t.insert(std::make_pair(ref new Cursor(curPos), ref new SNewPart));
	m_texts = reorginised;
	ControlText = newCtrlTxt;
}

SketchMusic::CompositionData::CompositionData()
{
	ControlText = ref new Text(ref new Instrument(SerializationTokens::CONTROL_TEXT));
	this->texts = ref new Platform::Collections::Vector < Text^ >;
	BPM = 120;
}

IJsonValue^ SketchMusic::Composition::Serialize()
{
	auto json = ref new JsonObject;
	json->Insert(SerializationTokens::PROJ_NAME, JsonValue::CreateStringValue(Header->Name));
	json->Insert(SerializationTokens::PROJ_BPM, JsonValue::CreateNumberValue(Data->BPM));
	json->Insert(SerializationTokens::PROJ_DESC, JsonValue::CreateStringValue(Header->Description));
	json->Insert(SerializationTokens::TEXTS_ARRAY, Data->serialize());
	return json;
}

Composition ^ SketchMusic::Composition::Deserialize(JsonObject ^ json)
{
	Composition^ comp = ref new Composition;
	comp->Header->Name = json->GetNamedString(SerializationTokens::PROJ_NAME, "");
	comp->Header->Description = json->GetNamedString(SerializationTokens::PROJ_DESC, "");
	auto texts = json->GetNamedArray(SerializationTokens::TEXTS_ARRAY, nullptr);
	if (texts)
	{
		comp->Data = CompositionData::deserialize(texts);
	}
	comp->Data->BPM = json->GetNamedNumber(SerializationTokens::PROJ_BPM, 120);
	return comp;
}

void SketchMusic::PartDefinition::OnPropertyChanged(Platform::String ^ propertyName)
{
	PropertyChanged(this, ref new PropertyChangedEventArgs(propertyName));
}
