#pragma once

#include "../SketchMusic.h"
#include "Key.h"
#include <string>

using namespace SketchMusic;

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
		{
			return (ref new SNote(key->value))->ToString();
		}
		case KeyType::relativeNote:
			result += "" + key->value + "r";
			break;
		case KeyType::genericNote:
			result += "(" + (key->value % 100) + "," + (abs(key->value) / 100) + ")";
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
			result += L"\uE75D";
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
		case KeyType::beat:
			if (key->value >= 0)
			{
				result += L"\ue893";
			}
			else
			{
				result += L"\ue892";
			}
			break;
		case KeyType::enter:
			result += L"\uE751";
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
			break;
		case KeyType::backspace:
			result += L"\ue750";
			break;
		case KeyType::eraser:
			result += L"\ue75c";
			break;
		case KeyType::control:
			result += "Ctrl";
			break;
		case KeyType::stop:
			result += L"\ue15b";
			break;
		case KeyType::tempo:
			result += L"\uE98F"; //23f3 - часики
			break;
		case KeyType::quantization:
			result += L"Q"; // \uE8D6 - значок мелодии
			break;
		case KeyType::hide:
			result = L"\uE92F";
			break;
		case KeyType::layout:
			result = L"\uEDA7";
			break;
		case KeyType::end:
			result = L"\uE81A"; //E769 - значок паузы
			break;
			// символы про запас
			// E892 - перемотка назад
			// E893 - перемотка вперёд
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