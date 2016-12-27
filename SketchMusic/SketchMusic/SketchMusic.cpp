// Class1.cpp
#include "pch.h"
#include "SketchMusic.h"

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
				//jval = obj->GetNamedValue("v");
				//if (jval)
				//{
				//	val = static_cast<int>(jval->GetNumber());
				//}
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

