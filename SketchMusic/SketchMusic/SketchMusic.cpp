﻿// Class1.cpp
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
				auto str = obj->GetNamedString(t::NOTE_VALUE, "");
				return ref new SNewPart(str);
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
	return ControlText->getParts();
}

CompositionData ^ SketchMusic::CompositionData::deserialize(Platform::String^ str)
{
	SketchMusic::CompositionData^ data = nullptr;
	auto json = ref new JsonArray();
	if (JsonArray::TryParse(str, &json))
	{
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

	// выполняются следующие сценарии:
	// - переставление частей местами
	// - удаление частей
	// - добавление частей
	// - "укорачивание" частей
	// - "удлинение" частей

	// Вместо того, чтобы каждый сценарий реализовывать в отдельности, сейчас при перестроении мы создаём новые тексты
	// и реорганизуем их в таком виде, в котором они должны быть

	// например, было: ч0 - пос 0 длина 2, ч1 - пос 2 длина 2, ч2 - пос 4 длина 8
	// стало: ч2 нпос 0, ч0 - нпос 8, ч1 - нпос 10 

	// проверка на то, что реорганизовывать ничего не надо
	bool ok = true;
	for (auto&& part : parts)
	{
		static int cur = 0;
		if (part->originalPos->Beat != cur)
		{
			ok = false;
			break;
		}
		cur += part->length;
	}

	if (ok) return;

	auto reorginised = ref new Platform::Collections::Vector<Text^>;
	for (auto&& oldText : m_texts)
	{
		Text^ newText = ref new Text(oldText->instrument);

		for (auto&& part : parts)
		{
			static int curPos = 0;
			// определяем граничные итераторы части в данном тексте
			auto orig = part->originalPos;
			auto leftBound = oldText->_t.lower_bound(orig);	// откуда начнём "вырезать" текст
			int rightBeat = leftBound->first->Beat + part->length;
			int dif = part->originalPos->Beat - curPos;
			Cursor^ rightPos = ref new Cursor(rightBeat + dif);
			auto rightBound = oldText->_t.lower_bound(rightPos);	// где закончим

			// переставляем символы из старого текста в новый
			for (auto iter = leftBound; iter != rightBound; iter++)
			{
				auto sym = iter->second;
				newText->_t.insert(std::make_pair(ref new Cursor (iter->first->Beat + dif,iter->first->Tick),sym));
			}

			curPos += part->length;
		}

		reorginised->Append(newText);
	}

	m_texts = reorginised;
}

SketchMusic::CompositionData::CompositionData()
{
	ControlText = ref new Text(ref new Instrument(SerializationTokens::CONTROL_TEXT));
	this->texts = ref new Platform::Collections::Vector < Text^ >;
}

void SketchMusic::Composition::Serialize()
{
	throw ref new Platform::NotImplementedException();
}
