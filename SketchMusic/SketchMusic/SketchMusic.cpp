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

bool SketchMusic::SNote::EQ(ISymbol^ second)
{
	if (second->GetSymType() != SymbolType::NOTE) return false;
	auto note = dynamic_cast<SNote^>(second);
	if (note)
	{
		if ((note->_val == _val) &&
			(note->_velocity == _velocity) &&
			(note->_voice == _voice)) return true;
	}

	return false;
}

Platform::String^ SketchMusic::SRNote::valToString(int val)
{
	return "0";
}

bool SketchMusic::SRNote::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::RNOTE) return false;
	auto note = dynamic_cast<SRNote^>(second);
	if (note)
	{
		if ((note->_val == _val) &&
			(note->_velocity == _velocity) &&
			(note->_voice == _voice)) return true;
	}

	return false;
}

Platform::String^ SketchMusic::SGNote::valToString(int val)
{
	return "0";
}

bool SketchMusic::SGNote::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::NOTE) return false;
	auto note = dynamic_cast<SGNote^>(second);
	if (note)
	{
		if ((note->_valX == _valX) &&
			(note->_valY == _valY) &&
			(note->_velocity == _velocity) &&
			(note->_voice == _voice)) return true;
	}

	return false;
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
				val = (int) obj->GetNamedNumber(t::NOTE_VALUE);
				vel = (int) obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = (int) obj->GetNamedNumber(t::NOTE_VOICE,0);
				return ref new SNote(val);
			}
			case SymbolType::RNOTE:
			{
				val = (int) obj->GetNamedNumber(t::NOTE_VALUE);
				vel = (int) obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = (int) obj->GetNamedNumber(t::NOTE_VOICE,0);
				return ref new SRNote(val);
			}
			case SymbolType::GNOTE:
			{
				val = (int) obj->GetNamedNumber(t::NOTE_VALUE);
				vel = (int) obj->GetNamedNumber(t::NOTE_VELOCITY,0);
				voi = (int) obj->GetNamedNumber(t::NOTE_VOICE,0);
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

void SketchMusic::PartDefinition::OnPropertyChanged(Platform::String ^ propertyName)
{
	//PropertyChanged(this, ref new PropertyChangedEventArgs(propertyName));
}

bool SketchMusic::SSpace::EQ(ISymbol ^ second)
{
	if (second->GetSymType() == SymbolType::SPACE) return true;

	return false;
}

bool SketchMusic::SAccent::EQ(ISymbol ^ second)
{
	if (second->GetSymType() == SymbolType::ACCENT) return true;

	return false;
}

bool SketchMusic::SClef::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::CLEF) return false;
	auto clef = dynamic_cast<SClef^>(second);
	if (clef)
	{
		if (clef->bottom == bottom) return true;
	}

	return false;
}

bool SketchMusic::SNewPart::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::NPART) return false;
	auto part = dynamic_cast<SNewPart^>(second);
	if (part)
	{
		if ((part->category == category) &&
			(part->dynamic == dynamic) &&
			(part->Name == Name)) return true;
	}

	return false;
}

bool SketchMusic::STempo::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::TEMPO) return false;
	auto tempo = dynamic_cast<STempo^>(second);
	if (tempo)
	{
		if (tempo->value == value) return true;
	}

	return false;
}

bool SketchMusic::SNoteEnd::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::END) return false;
	auto note = dynamic_cast<SNoteEnd^>(second);
	if (note)
	{
		if ((note->_val == _val) &&
			(note->_velocity == _velocity) &&
			(note->_voice == _voice)) return true;
	}

	return false;
}

bool SketchMusic::SChord::ModulatorsEQ(SChordMod m)
{
	if ((m.aug == mod.aug) &&
		(m.dim == mod.dim) &&
		(m.dom == mod.dom) &&
		(m.minor == mod.minor) &&
		(m._2 == mod._2) &&
		(m._2aug == mod._2aug) &&
		(m._2dim == mod._2dim) &&
		(m._3aug == mod._3aug) &&
		(m._3ex == mod._3ex) &&
		(m._4 == mod._4) &&
		(m._4aug == mod._4aug) &&
		(m._4dim == mod._4dim) &&
		(m._5ex == mod._5ex) &&
		(m._6 == mod._6) &&
		(m._6aug == mod._6aug) &&
		(m._6dim == mod._6dim) &&
		(m._7 == mod._7) &&
		(m._8 == mod._8) &&
		(m._9 == mod._9)) return true;
	return false;
}

bool SketchMusic::SChord::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::CHORD) return false;
	auto chord = dynamic_cast<SChord^>(second);
	if (chord)
	{
		if ((chord->key == key) &&
			(chord->bass == bass) &&
			(chord->degree == degree) &&
			(ModulatorsEQ(chord->mod)) &&
			(chord->variation == variation)) return true;
	}

	return false;
}

bool SketchMusic::SNewLine::EQ(ISymbol ^ second)
{
	if (second->GetSymType() == SymbolType::NLINE) return true;
	return false;
}

bool SketchMusic::SString::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::STRING) return false;
	auto str = dynamic_cast<SString^>(second);
	if (str)
	{
		if (str->value == value) return true;
	}
	return false;
}

//void SketchMusic::Commands::CommandManager::OnPropertyChanged(Platform::String ^ propertyName)
//{
//	this->PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
//}
