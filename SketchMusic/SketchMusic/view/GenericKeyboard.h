﻿//
// GenericKeyboard.h
// Объявление класса GenericKeyboard.
//

#pragma once

#include "../SketchMusic.h"
#include "Key.h"
#include <string>

using namespace Platform;
using namespace SketchMusic;
using namespace SketchMusic::View;

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
			result += "" + key->value + "g";
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
			result += L"\ue150";
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

public ref class SketchMusic::View::OnKeyToColorConverter sealed : Windows::UI::Xaml::Data::IValueConverter
{
public:
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
		case SketchMusic::View::KeyType::genericNote:
			break;
		default:
			return nullptr; // для остальных остаётся классический бэкграунд
		}

		Windows::UI::Xaml::Media::Brush^ brush = nullptr;

		switch (key->value % 12)
		{
		case 1: case 4: case 6: case 9: case 11:
		case -1: case -3: case -6: case -8: case -11:
			brush = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::DarkRed);
			break;
		default:
			brush = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::DarkOrange);
			break;
		}

		return brush;
	}

	// обращаю внимание, что при данном подходе не получится сделать convert и convertBack и получить тот же результат
	virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
	}

	OnKeyToColorConverter() {}
};

/**
TODO : нужно ли выделять квадкейбоард?
*/
public ref class SketchMusic::View::GenericKeyboard sealed : public Windows::UI::Xaml::Controls::Control, public SketchMusic::View::IKeyboard
{
private:
	bool input;
	//std::vector<SketchMusic::View::Key^> inputBuffer;
	int octaveModificator;

	void PushKey(Object^ object);
	void ReleaseKey(Object^ object);

	SketchMusic::View::KeyboardState^ currentState;	// состояние клавиатуры. Изменяется при нажатии всяких контрол, шифт и так далее

	int pressedKeys;	// суммарное количество нажатых клавиш
	

	std::multimap<SketchMusic::View::Key^, bool> _keys;	// сохраняем ссылки на клавиши, чтобы уведомлять их об изменении состояния
		// вторым параметром отмечаем, что клавиша в данный момент нажата

	concurrency::cancellation_token_source releaseToken;

protected:
	void OnKeyboardStateChanged(Object^ object, SketchMusic::View::KeyboardState ^ state);


public:
	GenericKeyboard();

	property bool recording;
	property bool cycling;
	property bool metronome;
	property bool control;

	virtual event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyPressed;	// нажатие на одну клавишу
	virtual event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyReleased;	// отпускание одной клавиши
	virtual event EventHandler<Windows::Foundation::Collections::IVector<SketchMusic::View::Key^>^>^ KeyboardReleased; // "отпускание" всех клавиш
	virtual event EventHandler<SketchMusic::View::KeyboardState^>^ KeyboardStateChanged;

	virtual void OnApplyTemplate() override;
	
	// для тача
	void onKeyboardControlEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
	void OnPointerExited(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	
	// для мыши
	void onKeyboardControlPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
	void OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnClosed(Platform::Object ^sender, Platform::Object ^args);
	void OnOpened(Platform::Object ^sender, Platform::Object ^args);
};
