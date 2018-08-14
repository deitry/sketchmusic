#pragma once

#include "../SketchMusic.h"

[Windows::UI::Xaml::Data::BindableAttribute]
public ref class SketchMusic::SNote sealed : public SketchMusic::ISymbol, public SketchMusic::INote
{
private:
	static Platform::String^ valToString(int val);	// то ли перенеси функцию в INote,
													// то ли вообще вынести в отдельный класс, который занимаетс€ преобразовани€ми
public:
	SNote() { _val = 0; }
	SNote(int tone) { _val = tone; _velocity = 0; _voice = 0; }
	SNote(int tone, int velocity, int voice) { _val = tone; _velocity = velocity; _voice = voice; }

	virtual property int _val;
	virtual property int _velocity;
	virtual property int _voice;

	virtual SymbolType GetSymType() { return SymbolType::NOTE; }
	virtual Platform::String^ ToString() {
		String^ str;
		switch (_val % 12)
		{
		case 0: str += "A"; break;
		case 1: case -11: str += "A#"; break;
		case 2: case -10: str += "B"; break;
		case 3: case -9: str += "C"; break;
		case 4: case -8: str += "C#"; break;
		case 5: case -7: str += "D"; break;
		case 6: case -6: str += "D#"; break;
		case 7: case -5: str += "E"; break;
		case 8: case -4: str += "F"; break;
		case 9: case -3: str += "F#"; break;
		case 10: case -2: str += "G"; break;
		case 11: case -1: str += "G#"; break;
		default: break;
		}
		str += ((_val + MIDIKEYTOA4 - 9) / 12 - 1);
		return str;
	}

	// ”наследовано через ISymbol
	virtual bool EQ(ISymbol^ second);
	//virtual Platform::String^ Serialize() { return "note," + valToString(_val); }	// приставка указывает на тип
};