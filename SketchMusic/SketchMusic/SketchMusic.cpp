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
			case SymbolType::SCALE:
		//	{
		//		jval = obj->GetNamedValue("v");
		//		if (jval)
		//		{
		//			val = static_cast<int>(jval->GetNumber());
		//		}
		//		return ref new SString(val);
		//	}
			case SymbolType::STRING:
		//	{
		//		String^ str;
		//		jval = obj->GetNamedValue("s");
		//		if (jval)
		//		{
		//			str = jval->GetString();
		//		}
		//		return ref new SString(str);
		//	}
			default: return nullptr;
			}
		}
	}
	return nullptr;
}

ISymbol ^ SketchMusic::ISymbolFactory::CreateSymbol(SketchMusic::View::KeyType type, Object^ val)
{
	//SymbolType sym_type = static_cast<SymbolType>(type);
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
	//case KeyType::scale:
		//	{
		//		jval = obj->GetNamedValue("v");
		//		if (jval)
		//		{
		//			val = static_cast<int>(jval->GetNumber());
		//		}
		//		return ref new SString(val);
		//	}
	//case SymbolType::STRING:
		//	{
		//		String^ str;
		//		jval = obj->GetNamedValue("s");
		//		if (jval)
		//		{
		//			str = jval->GetString();
		//		}
		//		return ref new SString(str);
		//	}
	default: return nullptr;
	}
	return nullptr;
}

/*ISymbol ^ SketchMusic::ISymbolFactory::CreateSymbol(int type, int val)
{
	switch (type)
	{
	NOTE:
		{
			return ref new SNote(val);
		}
	RNOTE:
		{
			return ref new SRNote(val);
		}
	GNOTE:
		{
			return ref new SGNote(val);
		}
	END:
		{
			return ref new SNoteEnd();
		}
	NLINE:
		{
			return ref new SNewLine();
		}
	SPACE:
		{
			return ref new SSpace();
		}
	//SCALE:
	//	{
	//		jval = obj->GetNamedValue("v");
	//		if (jval)
	//		{
	//			val = static_cast<int>(jval->GetNumber());
	//		}
	//		return ref new SString(val);
	//	}
	//STRING:
	//	{
	//		String^ str;
	//		jval = obj->GetNamedValue("strv");
	//		if (jval)
	//		{
	//			str = jval->GetString();
	//		}
	//		return ref new SString(str);
	//	}
	}
	return nullptr;
}*/

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
				data->texts->Append(text);
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
				data->texts->Append(text);
			}
		}
	}
	return data;
}

Windows::Data::Json::IJsonValue ^ SketchMusic::CompositionData::serialize()
{
	auto json = ref new JsonArray();
	for (auto&& text : texts)
	{
		if (text->getSize())
		{
			json->Append(static_cast<SketchMusic::Text^>(text)->serialize());
		}
	}
	return json;
}

SketchMusic::CompositionData::CompositionData()
{
	this->texts = ref new Platform::Collections::Vector < Text^ >;
}
