#include "pch.h"
#include "Composition.h"
#include "Text.h"
#include "STempo.h"

using namespace SketchMusic;
namespace t = SketchMusic::SerializationTokens;

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
	float curBpm = BPM;
	std::pair<Cursor^, ISymbol^> prevTempo;
	for (auto&& i : ControlText->_t)
	{
		if (i.second && i.second->GetSymType() == SymbolType::TEMPO)
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
		json->Append(static_cast<SketchMusic::Text^>(text)->serialize());
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
			for (unsigned int i = 0; i < m_texts->Size + 1; i++)
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

bool SketchMusic::CompositionData::HasContent()
{
	if (ControlText && ControlText->_t.size()) return true;
	for (auto text : texts)
	{
		if (text->_t.size()) return true;
	}
	return false;
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
	json->Insert(SerializationTokens::IDEAS_ARRAY, Lib->Serialize());
	return json;
}

CompositionLibrary ^ SketchMusic::CompositionLibrary::Deserialize(Windows::Data::Json::JsonArray ^ json)
{
	auto lib = ref new CompositionLibrary;
	for (auto i : json)
	{
		auto obj = i->GetObject();
		auto posIdea = SketchMusic::PositionedIdea::Deserialize(obj);
		if (posIdea)
		{
			lib->Ideas->Append(posIdea);
		}
	}
	return lib;
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
	auto ideas = json->GetNamedArray(SerializationTokens::IDEAS_ARRAY, nullptr);
	if (ideas)
	{
		comp->Lib = CompositionLibrary::Deserialize(ideas);
	}
	comp->Data->BPM = static_cast<float>(json->GetNamedNumber(SerializationTokens::PROJ_BPM, 120));
	return comp;
}

Windows::Data::Json::IJsonValue ^ SketchMusic::CompositionLibrary::Serialize()
{
	auto json = ref new JsonArray();
	for (auto&& idea : Ideas)
	{
		json->Append(static_cast<SketchMusic::PositionedIdea^>(idea)->Serialize());
	}
	return json;
}

