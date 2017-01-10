//
// BaseKeyboard.h
// Объявление класса BaseKeyboard.
//

#pragma once

#include "../SketchMusic.h"
#include "Key.h"
#include <string>

using namespace Platform;
using namespace SketchMusic;
using namespace SketchMusic::View;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml;

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::OnSymbolToStringConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		ISymbol^ sym = dynamic_cast<ISymbol^>(value);
		if (sym == nullptr) return "";

		INote^ note = dynamic_cast<INote^>(sym);
		if (note == nullptr) return "";

		String^ result = "" + note->_val;

		SGNote^ gen = dynamic_cast<SGNote^>(sym);
		if (gen)
		{
			result += "g";
		}

		SRNote^ rel = dynamic_cast<SRNote^>(sym);
		if (rel)
		{
			result += "r";
		}

		return result;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
		//return ref new SketchMusic::PositionedSymbol(ref new SketchMusic::Cursor, ref new SketchMusic::SNote);
	}

	OnSymbolToStringConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::OnKeyToStringConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	// на основе клавиши выдаёт её графическое отображение
	// TODO : для всякого рода перемещений, стрелочек и так далее сделать красиво
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(value);
		if (key == nullptr) return "";

		String^ result = "";

		switch (key->type)
		{
		case KeyType::note:
			result += "" + key->value;
			break;
		case KeyType::relativeNote:
			result += "" + key->value + "r";
			break;
		case KeyType::genericNote:
			result += "(" + (abs(key->value)/100) + "," + (key->value % 100) + ")";
			break;
		case KeyType::octave:
			if (key->value >= 0)
			{
				result += L"\ue109";
			}
			else
			{
				result += L"\ue108";
			}
			break;
		case KeyType::zoom:
			if (key->value >= 0)
			{
				result += L"\ue8a3";
			}
			else
			{
				result += L"\ue71f";
			}
			break;
		case KeyType::space:
			result += L"\u2423";
			break;
		case KeyType::move:
			if (key->value >= 0)
			{
				result += L"\ue111";
			}
			else
			{
				result += L"\ue112";
			}
			break;
		case KeyType::enter:
			result += L"\u21a9";
			break;
		case KeyType::play:
			if (key->value >= 0)
			{
				result += L"\ue102";
			}
			else
			{
				// reverse play
			}

			// \ue103 - пауза
			break;
		case KeyType::record:
			result += L"\ue1f5";
			break;
		case KeyType::cycling:
			result += L"\ue1cd";	// круговая стрелка
									//result += L"\ue1cс";	// однократный повтор
			break;
		case KeyType::deleteSym:
			result += L"\ue750";
			break;
		case KeyType::control:
			result += "Ctrl";
			break;
		case KeyType::stop:
			result += L"\ue15b";
			break;
		case KeyType::tempo:
			result += L"\u23f3";
			break;
		case KeyType::quantization:
			result += L"\uE8D6";
			break;
		case KeyType::hide:
			result = L"\uE92F";
			break;
		case KeyType::layout:
			result = L"\uEDA7";
			break;
		default:
			break;
		}

		return result;
	}

	// обращаю внимание, что при данном подходе не получится сделать convert и convertBack и получить тот же результат
	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		Platform::String^ str = dynamic_cast<Platform::String^>(value);
		if (str == nullptr) return nullptr;

		int num = std::stoi(str->Data());

		return ref new SketchMusic::View::Key(num);
	}

	OnKeyToStringConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::BoolToColorConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	property Windows::UI::Xaml::Media::Brush^ KeyBrush;
	property Windows::UI::Xaml::Media::Brush^ PressedKeyBrush;

	// на основе клавиши выдаёт её графическое отображение
	// TODO : для всякого рода перемещений, стрелочек и так далее сделать красиво
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		bool isEnabled = (bool)value;
		Windows::UI::Xaml::Media::Brush^ brush = isEnabled ? KeyBrush : PressedKeyBrush;

		return brush;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	BoolToColorConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::TagToKeyConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		Platform::String^ tag = (Platform::String^)value;
		if (tag == nullptr) return nullptr;

		int num = std::stoi(tag->Data());
		SketchMusic::View::Key^ key = ref new SketchMusic::View::Key(num);
		return key;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	TagToKeyConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::BlackKeyToColorConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	property Windows::UI::Xaml::Media::Brush^ BlackKeyBrush;

	// на основе клавиши выдаёт её графическое отображение
	// TODO : для всякого рода перемещений, стрелочек и так далее сделать красиво
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(value);
		if (key == nullptr) return nullptr;

		switch (key->type)
		{
		case SketchMusic::View::KeyType::note:
		case SketchMusic::View::KeyType::relativeNote:
			break;
		case SketchMusic::View::KeyType::genericNote:
		default:
			return nullptr; // для остальных остаётся классический бэкграунд
		}

		Windows::UI::Xaml::Media::Brush^ brush = nullptr;

		switch (key->value % 12)
		{
		case 1: case 4: case 6: case 9: case 11:
		case -1: case -3: case -6: case -8: case -11:
			brush = BlackKeyBrush;
			break;
		default:
			break;
		}

		return brush;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	BlackKeyToColorConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::KeyStateToColorConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	property Windows::UI::Xaml::Media::Brush^ KeyBrush;
	property Windows::UI::Xaml::Media::Brush^ PressedKeyBrush;

	// на основе клавиши выдаёт её графическое отображение
	// TODO : для всякого рода перемещений, стрелочек и так далее сделать красиво
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(value);
		if (key == nullptr) return nullptr;
		auto ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
		if (ctrl == nullptr) return nullptr;

		Windows::UI::Xaml::Media::Brush^ brush = nullptr;

		if (key->value)
		{
			brush = KeyBrush;
		}
		else
		{
			brush = PressedKeyBrush;
		}
		return brush;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	KeyStateToColorConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::OnPositionedSymbolToTextConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		SketchMusic::PositionedSymbol^ psym = dynamic_cast<SketchMusic::PositionedSymbol^>(value);
		if (psym == nullptr) return "";

		SketchMusic::STempo^ tempo = dynamic_cast<SketchMusic::STempo^>(psym->_sym);
		if (tempo)
		{
			return L"" + tempo->value;
		}

		SketchMusic::SGNote^ gnote = dynamic_cast<SketchMusic::SGNote^>(psym->_sym);
		if (gnote)
		{
			return L"" + gnote->_valX + "," + gnote->_valY;
		} 

		SketchMusic::INote^ note = dynamic_cast<SketchMusic::INote^>(psym->_sym);
		if (note)
		{
			return L"" + note->_val;
		}

		return L"";
	}
	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
		//return ref new SketchMusic::PositionedSymbol(ref new SketchMusic::Cursor, ref new SketchMusic::SNote);
	}

	OnPositionedSymbolToTextConverter() {}
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::PSymbolToVerticalPosConverter sealed : Windows::UI::Xaml::Data::IValueConverter, Windows::UI::Xaml::DependencyObject
{
public:
	property double LineSpacing;	// рассотояние между предполагаемыми линиями нотного стана
	property double AvailableHeight;
		
	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		//SketchMusic::ISymbol^ symbol = dynamic_cast<SketchMusic::ISymbol^>(value);
		//if (symbol == nullptr) return nullptr;
		auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(value);
		if (psym == nullptr) return nullptr;

		SketchMusic::INote^ note = dynamic_cast<SketchMusic::INote^>(psym->_sym);

		double scaling = AvailableHeight;
		double Y = 0;

		if (note)
		{
			// TODO : сделать расчёт похитрее, чтобы выглядело как на нотном стане
			// В том числе разрещить вылезать за пределы, увеличивая тем самым высоту строки
			// (чтобы строки не налезали друг на друга)
			Y = note->_val >= 0 ?
				-abs(note->_val) % 12 :
				-12 * ((note->_val % 12) != 0) + abs(note->_val) % 12;
			Y *= scaling / 12;
			Y += scaling - 5;
		}
		else
		{
			STempo^ tempo = dynamic_cast<STempo^>(psym->_sym);
			if (tempo)
			{
				Y = -0.6*scaling;
			}
		}

		return Y;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	PSymbolToVerticalPosConverter() {}
};


[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::MultiplicatedLengthConverter sealed : Windows::UI::Xaml::Data::IValueConverter, Windows::UI::Xaml::DependencyObject
{
public:
	property double Multiplicator;	// рассотояние между предполагаемыми линиями нотного стана
	
	property double Default;

	virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
	{
		if (Default)
			return (Default)*Multiplicator;
		return ((double)value)*Multiplicator;
	}

	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	MultiplicatedLengthConverter() {}
};
