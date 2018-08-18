// Class1.cpp
#include "pch.h"
#include "SketchMusic.h"
#include "base/Text.h"
#include "base/SNote.h"
#include "base/SRNote.h"
#include "base/SGNote.h"
#include "base/SNoteEnd.h"
#include "base/SSpace.h"
#include "base/STempo.h"
#include "base/SScale.h"
#include "base/PartDefinition.h"

using namespace SketchMusic;
using namespace Platform;
using namespace Windows::Data::Json;
using SketchMusic::SymbolType;
namespace t = SketchMusic::SerializationTokens;

namespace
{
	std::map<Degree, Platform::String^> degreeSerializationString =
	{
		{ Degree::I,	t::SCALE_1 },
		{ Degree::II,	t::SCALE_2 },
		{ Degree::III,	t::SCALE_3 },
		{ Degree::IV,	t::SCALE_4 },
		{ Degree::V,	t::SCALE_5 },
		{ Degree::VI,	t::SCALE_6 },
		{ Degree::VII,	t::SCALE_7 },
	};
}

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
			case SymbolType::SCALE:
			{
				auto scaleBase = static_cast<NoteType>(static_cast<int>(obj->GetNamedNumber(t::SCALE_BASE, 0)));
				auto steps = ref new Platform::Collections::Map<Degree, DegreeInfo>();
				auto readDegreeInfo = [steps, obj] (Degree degree)
				{
					auto degreeInfo = static_cast<DegreeInfo>(static_cast<int>(obj->GetNamedNumber(::degreeSerializationString.at(degree)), 
																								   static_cast<int>(DegreeInfo::Tone)));
					steps->Insert(degree, degreeInfo);
				};

				readDegreeInfo(Degree::I);
				readDegreeInfo(Degree::II);
				readDegreeInfo(Degree::III);
				readDegreeInfo(Degree::IV);
				readDegreeInfo(Degree::V);
				readDegreeInfo(Degree::VI);
				readDegreeInfo(Degree::VII);

				return ref new SScale(scaleBase, steps);
			}
			case SymbolType::HARMONY:
			{
				auto val = static_cast<int>(obj->GetNamedNumber(t::NOTE_VALUE, 1));
				return ref new SHarmony(val);
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
				auto val = obj->GetNamedNumber(t::NOTE_VALUE, 120.);
				return ref new STempo(static_cast<float>(val));
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

ISymbol ^ SketchMusic::ISymbolFactory::CreateSymbol(SketchMusic::View::KeyType type, 
													Object^ val)
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
	case SketchMusic::View::KeyType::scale:
	{
		int ival = (int)val;
		auto baseNote = static_cast<NoteType>(ival);
		return ref new SScale(baseNote, ScaleCategory::Major);
	}
	case SketchMusic::View::KeyType::harmony:
	{
		int ival = (int)val;
		return ref new SHarmony(ival);
	}
	case SketchMusic::View::KeyType::newPart:
	{
		auto str = (Platform::String^)val;
		return ref new SNewPart(str);
	}
	case SketchMusic::View::KeyType::tempo:
	{
		auto ival = 120.f;
		if (val) ival = static_cast<float>(safe_cast<int>(val));
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

bool SketchMusic::SHarmony::EQ(ISymbol ^ second)
{
	if (second->GetSymType() != SymbolType::HARMONY) return false;
	auto rhs = dynamic_cast<SHarmony^>(second);
	if (rhs)
	{
		if (rhs->_val == _val) return true;
	}
	return false;
}

Platform::String^ SketchMusic::noteTypeToString(NoteType note)
{
	switch (note)
	{
		case NoteType::A:	return "A";
		case NoteType::As:	return "As";
		case NoteType::B:	return "B";
		case NoteType::C:	return "C";
		case NoteType::Cs:	return "Cs";
		case NoteType::D:	return "D";
		case NoteType::Ds:	return "Ds";
		case NoteType::E:	return "E";
		case NoteType::F:	return "F";
		case NoteType::Fs:	return "Fs";
		case NoteType::G:	return "G";
		case NoteType::Gs:	return "Gs";
		default: break;
	}
	return "?";
}

Platform::String ^ SketchMusic::SHarmony::ToString()
{
	Platform::String^ str = "";
	Platform::String^ sharp = "s";
	switch (_val % 12)
	{
	case 0: str += "I"; break;
	case 1: case -11: str += "I" + sharp; break;
		case 2: case -10: str += "II"; break;
		case 3: case -9 :str += "III"; break;
		case 4: case -8: str += "III" + sharp; break;
		case 5: case -7: str += "IV"; break;
		case 6: case -6: str += "IV" + sharp; break;
		case 7: case -5: str += "V"; break;
		case 8: case -4: str += "VI"; break;
		case 9: case -3: str += "VI" + sharp; break;
		case 10: case -2: str += "VII"; break;
		case 11: case -1: str += "VII" + sharp; break;
		default: break;
	}

	auto oct = _val / 12;
	if (oct > 0)
	{
		str += "+" + oct;
	}
	else if (_val < 0)
	{
		str += "-" + abs(oct - (_val % 12 ? 1 : 0));
	}

	return str;
};