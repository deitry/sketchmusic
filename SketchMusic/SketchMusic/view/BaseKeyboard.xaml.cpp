//
// BaseKeyboard.cpp
// Реализация класса BaseKeyboard.
//

#include "pch.h"
#include "BaseKeyboard.xaml.h"
#include <string>

using namespace SketchMusic::View;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

// Документацию по шаблону элемента "Элемент управления на основе шаблона" см. по адресу http://go.microsoft.com/fwlink/?LinkId=234235

SketchMusic::View::BaseKeyboard::BaseKeyboard()
{
	InitializeComponent();

	_dict = this->Resources;
	
	currentState = ref new KeyboardState(KeyboardStateEnum::normal);
	_layout = KeyboardType::Basic;

	InitializePage();

	// TODO ? переделать на КвадКейборд?
	// и в зависимости от параметра - какой тип клавиатуры нужен, в случае больших / бесконечных
	// (как в случае "гитарной", "баянной" и т.д.) создавать динамически
	this->KeyboardStateChanged += ref new Windows::Foundation::EventHandler<SketchMusic::View::KeyboardState ^>(this, &SketchMusic::View::BaseKeyboard::OnKeyboardStateChanged);
}

inline void UpdateParent(SketchMusic::View::Key^ key)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
	if (ctrl == nullptr)
		return;
	ctrl->Content = key;
}

void SketchMusic::View::BaseKeyboard::OnKeyboardPressed(SketchMusic::View::Key^ key)
{
	switch (key->type)
	{
	case SketchMusic::View::KeyType::note:
	case SketchMusic::View::KeyType::genericNote:
	case SketchMusic::View::KeyType::relativeNote:
		break;
	default:
	{
		// использовать конвертер тип состояния -> стиль кнопки
		auto ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
		if (ctrl) ctrl->IsEnabled = false;
		break;
	}
	}
}

void SketchMusic::View::BaseKeyboard::OnControlPressed(SketchMusic::View::Key ^ key)
{
	switch (key->type)
	{
	case KeyType::control:
		key->value = !key->value;
		UpdateParent(key);
		break;
	case KeyType::octave:
		key->type = KeyType::zoom;
		UpdateParent(key);
		break;
	case KeyType::zoom:
		key->type = KeyType::octave;
		UpdateParent(key);
		break;
	}
}

inline void OnOctaveInc(SketchMusic::View::Key^ key)
{
	switch (key->type)
	{
	case SketchMusic::View::KeyType::note:
	case SketchMusic::View::KeyType::genericNote:
	case SketchMusic::View::KeyType::relativeNote:
		key->value += 12;
		UpdateParent(key);
		break;
	default:
		break;
	}
}

inline void OnOctaveDec(SketchMusic::View::Key^ key)
{
	switch (key->type)
	{
	case SketchMusic::View::KeyType::note:
	case SketchMusic::View::KeyType::genericNote:
	case SketchMusic::View::KeyType::relativeNote:
		key->value -= 12;
		UpdateParent(key);
		break;
	default:
		break;
	}
}

void SketchMusic::View::BaseKeyboard::OnNormalState(SketchMusic::View::Key^ key)
{
	switch (key->type)
	{
	case SketchMusic::View::KeyType::note:
	case SketchMusic::View::KeyType::relativeNote:
	case SketchMusic::View::KeyType::genericNote:
		break;
	default:
	{
		// использовать конвертер тип состояния -> стиль кнопки
		auto ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
		if (ctrl) ctrl->IsEnabled = true;
		break;
	}
	}
}

void SketchMusic::View::BaseKeyboard::OnPlayStopState(SketchMusic::View::Key ^ key)
{
	switch (key->type)
	{
	case SketchMusic::View::KeyType::play:
		key->type = SketchMusic::View::KeyType::stop;
		key->value = 1;
		UpdateParent(key);
		break;
	case SketchMusic::View::KeyType::stop:
		key->type = SketchMusic::View::KeyType::play;
		key->value = 0;
		UpdateParent(key);
		break;
	}
}

void SketchMusic::View::BaseKeyboard::OnKeyboardStateChanged(Object^ object, SketchMusic::View::KeyboardState ^ state)
{
	// проходимся по всем кнопкам и меняем их состояние согласно их типу
	for (auto&& pkey : _keys)
	{
		auto key = pkey.first;
		switch (state->state)
		{
		case SketchMusic::View::KeyboardStateEnum::control:
			OnControlPressed(key);
			break;
		case SketchMusic::View::KeyboardStateEnum::pressed:
			OnKeyboardPressed(key);
			break;
		case SketchMusic::View::KeyboardStateEnum::octaveInc:
			OnOctaveInc(key);
			break;
		case SketchMusic::View::KeyboardStateEnum::octaveDec:
			OnOctaveDec(key);
			break;
		case SketchMusic::View::KeyboardStateEnum::normal:
			OnNormalState(key);
			break;
		case SketchMusic::View::KeyboardStateEnum::play:
		case SketchMusic::View::KeyboardStateEnum::stop:
			OnPlayStopState(key);
			break;
		}
	}
}

/**
Нажатие на одну из клавиш клавиатуры
*/
void SketchMusic::View::BaseKeyboard::PushKey(Object^ sender)
{
	pressedKeys++;

	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if (ctrl)
	{
		SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(ctrl->Content);

		KeyboardEventArgs^ args = ref new KeyboardEventArgs(key, this->pressedKeys);
		ctrl->Background = (SolidColorBrush^)_dict->Lookup("draggedForegroundBrush");

		if (key)
		{
			switch (key->type)
			{
			case SketchMusic::View::KeyType::note:
			case SketchMusic::View::KeyType::genericNote:
			case SketchMusic::View::KeyType::relativeNote:
				KeyPressed(this, args);
				if (pressedKeys == 1)
				{
					currentState->state = SketchMusic::View::KeyboardStateEnum::pressed;
					KeyboardStateChanged(this, currentState);
				}
				break;
			case SketchMusic::View::KeyType::octave:
				this->currentState->state = (key->value > 0)
					? KeyboardStateEnum::octaveInc
					: KeyboardStateEnum::octaveDec;
				KeyboardStateChanged(this, currentState);
				break;
			case SketchMusic::View::KeyType::zoom:
				KeyPressed(this, args);
				break;
			case SketchMusic::View::KeyType::record:
				key->value = !key->value;
				UpdateParent(key);

				KeyPressed(this, args);
				break;
			case SketchMusic::View::KeyType::control:
				this->currentState->state = KeyboardStateEnum::control;	// TODO : выглядит очень плохо.
																		// Надо будет понять, что вообще имеется в виду под state
				KeyboardStateChanged(this, currentState);
				break;
			case SketchMusic::View::KeyType::cycling:
				key->value = !key->value;
				UpdateParent(key);

				KeyPressed(this, args);
				break;

			case SketchMusic::View::KeyType::layout:
			case SketchMusic::View::KeyType::tempo:
			case SketchMusic::View::KeyType::quantization:
			case SketchMusic::View::KeyType::hide:
				// обработка этих клавиш идёт, когда мы отпускаем клавишу, а не нажимаем. Вызывать событие нажатия не требуется
				break;
			
			case SketchMusic::View::KeyType::enter:
			case SketchMusic::View::KeyType::play:
			case SketchMusic::View::KeyType::move:
			case SketchMusic::View::KeyType::space:
			case SketchMusic::View::KeyType::deleteSym:
			default:
				KeyPressed(this, args);
				break;
			}
		}
	}
}

void SketchMusic::View::BaseKeyboard::onKeyboardControlPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// нажатие мышкой - только для нот
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	PushKey(sender);
	//mousePressed = true;
	auto el = dynamic_cast<Windows::UI::Xaml::FrameworkElement^>(sender);
	el->CapturePointer(e->Pointer);
}

void SketchMusic::View::BaseKeyboard::onKeyboardControlEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// нажатие тачем или мышкой, если она не отжималась
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		//&& (!mousePressed)	// оставим на будущее, когда я придумаю, как сделать отпускание.
		// а может, эта штука вовсе не нужна
		)
		return;

	PushKey(sender);
}

void SketchMusic::View::BaseKeyboard::InitializePage()
{
	if (mainPanel == nullptr) return;

	for (auto&& row : mainPanel->Children)
	{
		StackPanel^ stackPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(row));
		if (stackPanel == nullptr)
			continue;

		for (auto&& element : stackPanel->Children)
		{
			ContentControl^ ctrl = dynamic_cast<ContentControl^>(static_cast<Object^>(element));
			if (ctrl)
			{
				SketchMusic::View::Key^ key = dynamic_cast<Key^>(ctrl->Content);
				if (key)
				{
					key->parent = ctrl;
					_keys.insert(std::make_pair(key, false));
				}
				
				// позже для оптимизации можно будет часть клавиш держать в одном массиве, часть в другом
				// сейчас ещё не до конца понятно, какие клавиши во что будем превращать

				// для тача
				ctrl->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::BaseKeyboard::onKeyboardControlEntered);
				ctrl->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::BaseKeyboard::OnPointerExited);

				// для мыши	
				ctrl->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::BaseKeyboard::onKeyboardControlPressed);
				ctrl->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::BaseKeyboard::OnPointerReleased);
			}
		}
	}

	// специально проверяем на тот случай если курсор убегает за пределы клавиш и мы не знаем, отпустили его или нет
	//grid->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::BaseKeyboard::OnPointerReleased);
}


void SketchMusic::View::BaseKeyboard::OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	//mousePressed = false;
	// TODO : отключать и тогда, если "отжали" мышку вне гридов и контролов

	// отпускаем клавишу, если это не тач
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	ReleaseKey(sender);
}

void SketchMusic::View::BaseKeyboard::OnPointerExited(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	// отпускаем клавишу, если это тач или если мышь нажата и не отпускалась
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		//&& (!mousePressed)
		)
		return;

	ReleaseKey(sender);
}


void SketchMusic::View::BaseKeyboard::ReleaseKey(Object^ sender)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl =
		dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(static_cast<Object^>(sender));


	SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(ctrl->Content);
	if (key == nullptr)
		return;

	KeyboardEventArgs^ args = ref new KeyboardEventArgs(key, this->pressedKeys);

	if (ctrl)
	{
		ctrl->Background = nullptr;

		if (key)
		{
			switch (key->type)
			{
			case SketchMusic::View::KeyType::tempo:
			{
				auto flyout = Windows::UI::Xaml::Controls::Flyout::GetAttachedFlyout(ctrl);
				if (flyout)
					flyout->ShowAt(ctrl);
				break;
			}
			case SketchMusic::View::KeyType::quantization:
			{
				auto flyout = Windows::UI::Xaml::Controls::Flyout::GetAttachedFlyout(ctrl);
				if (flyout)
					flyout->ShowAt(ctrl);
				break;
			}
			case SketchMusic::View::KeyType::hide:
			{
				key->value = !key->value;
				UpdateParent(key);

				for (auto&& row : mainPanel->Children)
				{
					FrameworkElement^ el = dynamic_cast<FrameworkElement^>(static_cast<Object^>(row));
					if (el)
					{
						auto tag = ((String^)el->Tag);
						if (tag == L"CtrlPanel") continue;
						auto tagType = static_cast<KeyboardType>(std::stoi(tag->Data()));

						if (key->value == 0 && tagType == _layout) row->Visibility = Windows::UI::Xaml::Visibility::Visible;
						else row->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
					}
				}
				KeyReleased(this, args);
				break;
			}
			case SketchMusic::View::KeyType::layout:
			{
				// смена раскладки
				auto flyout = Windows::UI::Xaml::Controls::Flyout::GetAttachedFlyout(ctrl);
				if (flyout)
					flyout->ShowAt(ctrl);
				break;
			}
			case KeyType::note:
			case KeyType::relativeNote:
			case KeyType::genericNote:
				KeyReleased(this, args);
				break;
			default:
				KeyReleased(this, args);
			}
		}
	}

	pressedKeys--;

	if (pressedKeys <= 0)
	{
		pressedKeys = 0;	// на всякий случай принудительно

							// уведомляем всех о том, что всё норм
		this->currentState->state = SketchMusic::View::KeyboardStateEnum::normal;
		KeyboardStateChanged(this, currentState);
	}
}


void SketchMusic::View::BaseKeyboard::OnClosed(Platform::Object ^sender, Platform::Object ^args)
{
	Flyout^ flyout = dynamic_cast<Flyout^>(sender);
	if (flyout == nullptr)
		return;

	StackPanel^ panel = dynamic_cast<StackPanel^>(flyout->Content);

	for (auto&& child : panel->Children)
	{
		if (tempoPressed)
		{
			Slider^ slider = dynamic_cast<Slider^>(static_cast<Object^>(child));
			if (slider)
			{
				auto key = ref new SketchMusic::View::Key(KeyType::tempo, slider->Value);
				auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
				KeyPressed(this, args);
				tempoPressed = false;
			}
		}
		CheckBox^ checkBox = dynamic_cast<CheckBox^>(static_cast<Object^>(child));
		if (checkBox)
		{
			auto key = ref new SketchMusic::View::Key(KeyType::metronome, (int)checkBox->IsChecked->Value);
			auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
			KeyPressed(this, args);
		}
	}
}


void SketchMusic::View::BaseKeyboard::OnOpened(Platform::Object ^sender, Platform::Object ^args)
{

}


void SketchMusic::View::BaseKeyboard::OnClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
	tempoPressed = true;
	TempoFlyout->Hide();
}

void SketchMusic::View::BaseKeyboard::OnQuantizeClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	// 
	auto bt = dynamic_cast<Button^>(e->OriginalSource);
	if (bt)
	{
		quantizeNeed->IsChecked = true;

		// на сколько надо поделить
		int div = std::stoi(((String^)bt->Tag)->Data());

		auto key = ref new SketchMusic::View::Key(KeyType::quantization, div);
		auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
		KeyPressed(this, args);

		QuantizeFlyout->Hide();
	}
}


void SketchMusic::View::BaseKeyboard::quantizeNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	quantizeNeed->IsChecked = !quantizeNeed->IsChecked;
	auto key = ref new SketchMusic::View::Key(KeyType::quantization, 0);	// 0 - опасно, но хочу попробовать
	auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, args);
}


void SketchMusic::View::BaseKeyboard::QuantizeFlyout_Closed(Platform::Object^ sender, Platform::Object^ e)
{

}

/*
*/



void SketchMusic::View::BaseKeyboard::MenuFlyoutItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto el = dynamic_cast<FrameworkElement^>(e->OriginalSource);
	if (el)
	{
		int type = std::stoi(((String^)el->Tag)->Data());
		_layout = static_cast<KeyboardType>(type);

		for (auto&& row : mainPanel->Children)
		{
			auto el = dynamic_cast<FrameworkElement^>(static_cast<Object^>(row));
			if (el)
			{
				auto str = (String^)el->Tag;
				if (str == "CtrlPanel") continue;

				int type = std::stoi(str->Data());
				KeyboardType tag = static_cast<KeyboardType>(type);
				if (tag == _layout) el->Visibility = Windows::UI::Xaml::Visibility::Visible;
				else el->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			}
		}
	}

	LayoutFlyout->Hide();
}
