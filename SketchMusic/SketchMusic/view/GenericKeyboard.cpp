//
// GenericKeyboard.cpp
// Реализация класса GenericKeyboard.
//

#include "pch.h"
#include "GenericKeyboard.h"

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

GenericKeyboard::GenericKeyboard()
{
	DefaultStyleKey = "SketchMusic.View.GenericKeyboard";
	recording = false;
	currentState = ref new KeyboardState(KeyboardStateEnum::normal);

	// TODO ? переделать на КвадКейборд?
	// и в зависимости от параметра - какой тип клавиатуры нужен, в случае больших / бесконечных
	// (как в случае "гитарной", "баянной" и т.д.) создавать динамически
	this->KeyboardStateChanged += ref new Windows::Foundation::EventHandler<SketchMusic::View::KeyboardState ^>(this, &SketchMusic::View::GenericKeyboard::OnKeyboardStateChanged);
	// TODO : обработка клавиш клавиатуры
}

inline void UpdateParent(SketchMusic::View::Key^ key)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
	if (ctrl == nullptr)
		return;
	ctrl->Content = key;
}

inline void OnKeyboardPressed(SketchMusic::View::Key^ key)
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
			ctrl->IsEnabled = false;
			ctrl->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Gray);
			break;
		}
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

inline void OnNormalState(SketchMusic::View::Key^ key)
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
			ctrl->IsEnabled = true;
			ctrl->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::DarkOrange);
			break;
		}
	}
}

void SketchMusic::View::GenericKeyboard::OnKeyboardStateChanged(Object^ object, SketchMusic::View::KeyboardState ^ state)
{
	// проходимся по всем кнопкам и меняем их состояние согласно их типу
	for (auto&& pkey : _keys)
	{
		auto key = pkey.first;
		switch (state->state)
		{
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
		}	
	}
}

/**
Нажатие на одну из клавиш клавиатуры
*/
void SketchMusic::View::GenericKeyboard::PushKey(Object^ sender)
{
	pressedKeys++;

	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if (ctrl)
	{
		ctrl->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::AntiqueWhite);
		// по тегу мы должны понять, что теперь делать
		// цифры - нота
		// del - удалить ноту
		// ...
		// может быть вообще что угодно, поэтому нужно сделать подобие key со значением клавиши
		// и передавать её 
		SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(ctrl->Content);

		auto send = ref new SketchMusic::View::Key(key->type, key->value);
		KeyboardEventArgs^ args = ref new KeyboardEventArgs;
		args->key = send;
		args->keysPressed = this->pressedKeys;

		if (key)
		{
			switch (key->type)
			{
			case SketchMusic::View::KeyType::note:
			case SketchMusic::View::KeyType::genericNote:
			case SketchMusic::View::KeyType::relativeNote:
				args->key->value += octaveModificator * 12;
				KeyPressed(this, args);
				if (pressedKeys == 1)
				{
					currentState->state = SketchMusic::View::KeyboardStateEnum::pressed;
					KeyboardStateChanged(this, currentState);
				}
				break;
			case SketchMusic::View::KeyType::octave:
				//this->octaveModificator += key->value;
				if (!control)
				{
					// в обычном режиме меняем октавы
					this->currentState->state = (key->value > 0)
						? KeyboardStateEnum::octaveInc
						: KeyboardStateEnum::octaveDec;
					KeyboardStateChanged(this, currentState);
				}
				else
				{
					// если нажат контрол - зуммируем
					args->key->type = SketchMusic::View::KeyType::zoom;
					KeyPressed(this, args);
				}
				break;
			case SketchMusic::View::KeyType::record:
				recording = !recording;
				ctrl->Foreground = recording 
					? ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red)
					: ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
				args->key->value = recording ? 1 : 0;
				KeyPressed(this, args);
				break;
			case SketchMusic::View::KeyType::control:
				// TODO : менять состояние на ctrl
				// уведомлять об этом кнопки? чтобы они меняли своё состояние
				control = !control;
				ctrl->Foreground = control
					? ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Blue)
					: ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
				args->key->value = control ? 1 : 0;
				KeyPressed(this, args);
				break;
			case SketchMusic::View::KeyType::cycling:
				cycling = !cycling;
				ctrl->Foreground = cycling
					? ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Blue)
					: ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
				args->key->value = cycling ? 1 : 0;
				KeyPressed(this, args);
				break;
			case SketchMusic::View::KeyType::tempo:
			{
				auto flyout = Flyout::FlyoutBase::GetAttachedFlyout(ctrl);
				flyout->Opened += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &SketchMusic::View::GenericKeyboard::OnOpened);
				flyout->Closed += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &SketchMusic::View::GenericKeyboard::OnClosed);
				flyout->ShowAt(ctrl);
				break;
			}
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

void SketchMusic::View::GenericKeyboard::onKeyboardControlPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// нажатие мышкой - только для нот
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;
	
	PushKey(sender);
	//mousePressed = true;
	auto el = dynamic_cast<Windows::UI::Xaml::FrameworkElement^>(sender);
	el->CapturePointer(e->Pointer);
}

void SketchMusic::View::GenericKeyboard::onKeyboardControlEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// нажатие тачем или мышкой, если она не отжималась
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		//&& (!mousePressed)	// оставим на будущее, когда я придумаю, как сделать отпускание.
			// а может, эта штука вовсе не нужна
		)
		return;

	PushKey(sender);
}

void SketchMusic::View::GenericKeyboard::OnApplyTemplate()
{
	StackPanel^ mainPanel = (StackPanel^)GetTemplateChild("_stackPanel");
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
				int num = std::stoi(((String^)ctrl->Tag)->Data());
				SketchMusic::View::Key^ key = ref new SketchMusic::View::Key(num);
				ctrl->Content = key;
				key->parent = ctrl;
				_keys.insert(std::make_pair(key, false));

				// позже для оптимизации можно будет часть клавиш держать в одном массиве, часть в другом
				// сейчас ещё не до конца понятно, какие клавиши во что будем превращать

				// для тача
				ctrl->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::GenericKeyboard::onKeyboardControlEntered);
				ctrl->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::GenericKeyboard::OnPointerExited);
				

				// для мыши	
				ctrl->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::GenericKeyboard::onKeyboardControlPressed);
				ctrl->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::GenericKeyboard::OnPointerReleased);
				// как?- подписать ctrl на событие изменения состояния клавиатуры

			}
		}
	}
	// специально проверяем на тот случай если курсор убегает за пределы клавиш и мы не знаем, отпустили его или нет
	//grid->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::GenericKeyboard::OnPointerReleased);

	// находим Flyout, который управляет темпом
	// и связываем значение темпа и ползунок слайдера
	// или хотя бы вешаем на ползунок обработчик ValueChanged и не забываем при показе Flyout устанавливать текущее значение темпа
}


void SketchMusic::View::GenericKeyboard::OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	//mousePressed = false;
	// TODO : отключать и тогда, если "отжали" мышку вне гридов и контролов

	// отпускаем клавишу, если это не тач
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	ReleaseKey(sender);
}

void SketchMusic::View::GenericKeyboard::OnPointerExited(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	// отпускаем клавишу, если это тач или если мышь нажата и не отпускалась
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		//&& (!mousePressed)
		)
		return;

	ReleaseKey(sender);
}


void SketchMusic::View::GenericKeyboard::ReleaseKey(Object^ sender)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(static_cast<Object^>(sender));
	if (ctrl)
	{
		ctrl->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::DarkOrange);
	}
	
	pressedKeys--;

	SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(ctrl->Content);
	if (key == nullptr)
		return;

	KeyboardEventArgs^ args = ref new KeyboardEventArgs;
	args->key = key;
	args->keysPressed = this->pressedKeys;

	KeyReleased(this, args);

	if (pressedKeys <= 0)
	{
		pressedKeys = 0;	// на всякий случай принудительно

		// уведомляем всех о том, что всё норм
		this->currentState->state = SketchMusic::View::KeyboardStateEnum::normal;
		KeyboardStateChanged(this, currentState);
	}
}


void SketchMusic::View::GenericKeyboard::OnClosed(Platform::Object ^sender, Platform::Object ^args)
{
	// отправляем выбранное значение темпа
	Flyout^ flyout = dynamic_cast<Flyout^>(sender);
	if (flyout == nullptr)
		return;
	
	StackPanel^ panel = dynamic_cast<StackPanel^>(flyout->Content);
	for (auto&& child : panel->Children)
	{
		Slider^ slider = dynamic_cast<Slider^>(static_cast<Object^>(child));
		if (slider)
		{
			auto key = ref new SketchMusic::View::Key(KeyType::tempo, slider->Value);
			auto args = ref new KeyboardEventArgs;
			args->key = key;
			args->keysPressed = this->pressedKeys;
			KeyPressed(this, args);
		}
		CheckBox^ checkBox = dynamic_cast<CheckBox^>(static_cast<Object^>(child));
		if (checkBox)
		{
			auto key = ref new SketchMusic::View::Key(KeyType::metronome, (int)checkBox->IsChecked->Value);
			auto args = ref new KeyboardEventArgs;
			args->key = key;
			args->keysPressed = this->pressedKeys;
			KeyPressed(this, args);
		}
	}
}


void SketchMusic::View::GenericKeyboard::OnOpened(Platform::Object ^sender, Platform::Object ^args)
{
	
}
