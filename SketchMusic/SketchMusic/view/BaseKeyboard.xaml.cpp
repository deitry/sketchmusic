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
	this->KeyboardStateChanged += ref new Windows::Foundation::EventHandler<KeyboardState ^>(this, &BaseKeyboard::OnKeyboardStateChanged);
}

inline void UpdateParent(Key^ key)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(key->parent);
	if (ctrl == nullptr)
		return;
	ctrl->Content = key;
}

void SketchMusic::View::BaseKeyboard::OnKeyboardPressed(Key^ key)
{
	switch (key->type)
	{
	case KeyType::note:
	case KeyType::genericNote:
	case KeyType::relativeNote:
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

void SketchMusic::View::BaseKeyboard::OnControlPressed(Key ^ key)
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
	//case KeyType::deleteSym:
	//	key->type = KeyType::eraser;
	//	UpdateParent(key);
	//	break;
	//case KeyType::eraser:
	//	key->type = KeyType::deleteSym;
	//	key->value = 0;
	//	UpdateParent(key);
	//	break;
	}
}

inline void OnOctaveInc(Key^ key)
{
	switch (key->type)
	{
	case KeyType::note:
	//case KeyType::genericNote:
	case KeyType::relativeNote:
		key->value += 12;
		UpdateParent(key);
		break;
	default:
		break;
	}
}

inline void OnOctaveDec(Key^ key)
{
	switch (key->type)
	{
	case KeyType::note:
	//case KeyType::genericNote:
	case KeyType::relativeNote:
		key->value -= 12;
		UpdateParent(key);
		break;
	default:
		break;
	}
}

void SketchMusic::View::BaseKeyboard::OnNormalState(Key^ key)
{
	switch (key->type)
	{
	case KeyType::note:
	case KeyType::relativeNote:
	case KeyType::genericNote:
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

void SketchMusic::View::BaseKeyboard::OnPlayStopState(Key ^ key)
{
	switch (key->type)
	{
	case KeyType::play:
		key->type = KeyType::stop;
		key->value = 1;
		UpdateParent(key);
		break;
	case KeyType::stop:
		key->type = KeyType::play;
		key->value = 0;
		UpdateParent(key);
		break;
	}
}

void SketchMusic::View::BaseKeyboard::OnKeyboardStateChanged(Object^ object, KeyboardState ^ state)
{
	// проходимся по всем кнопкам и меняем их состояние согласно их типу
	for (auto&& pkey : _keys)
	{
		auto key = pkey.first;
		switch (state->state)
		{
		case KeyboardStateEnum::control:
			OnControlPressed(key);
			break;
		case KeyboardStateEnum::pressed:
			OnKeyboardPressed(key);
			break;
		case KeyboardStateEnum::octaveInc:
			OnOctaveInc(key);
			break;
		case KeyboardStateEnum::octaveDec:
			OnOctaveDec(key);
			break;
		case KeyboardStateEnum::normal:
			OnNormalState(key);
			break;
		case KeyboardStateEnum::play:
		case KeyboardStateEnum::stop:
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
		Key^ key = dynamic_cast<Key^>(ctrl->Content);
		OnPushKey(key);
	}
}

void SketchMusic::View::BaseKeyboard::OnPushKey(Key ^ key)
{
	if (key == nullptr) return;
	KeyboardEventArgs^ args = ref new KeyboardEventArgs(key, this->pressedKeys);
	((ContentControl^)key->parent)->Background = (SolidColorBrush^)_dict->Lookup("draggedForegroundBrush");

	if (key)
	{
		switch (key->type)
		{
		case KeyType::note:
		case KeyType::genericNote:
		case KeyType::relativeNote:
			KeyPressed(this, args);
			if (pressedKeys == 1)
			{
				currentState->state = KeyboardStateEnum::pressed;
				KeyboardStateChanged(this, currentState);
			}
			break;
		case KeyType::octave:
			this->currentState->state = (key->value > 0)
				? KeyboardStateEnum::octaveInc
				: KeyboardStateEnum::octaveDec;
			KeyboardStateChanged(this, currentState);
			break;
		case KeyType::zoom:
			KeyPressed(this, args);
			break;

			// переключатели
		case KeyType::eraser:
		case KeyType::cycling:
		case KeyType::record:
			key->value = !key->value;
			UpdateParent(key);

			KeyPressed(this, args);
			break;
		case KeyType::control:
			this->currentState->state = KeyboardStateEnum::control;	// TODO : выглядит очень плохо.
																	// Надо будет понять, что вообще имеется в виду под state
			KeyboardStateChanged(this, currentState);
			break;

		case KeyType::layout:
		case KeyType::tempo:
		case KeyType::quantization:
		case KeyType::hide:
			// обработка этих клавиш идёт, когда мы отпускаем клавишу, а не нажимаем. Вызывать событие нажатия не требуется
			break;

		case KeyType::enter:
			break;
		case KeyType::play:
		case KeyType::move:
		case KeyType::space:
		case KeyType::deleteSym:
		default:
			KeyPressed(this, args);
			break;
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
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen)
		//&& (!mousePressed)	// оставим на будущее, когда я придумаю, как сделать отпускание.
		// а может, эта штука вовсе не нужна
		)
		return;

	PushKey(sender);
}

void SketchMusic::View::BaseKeyboard::InitializePage()
{
	//this->TabNavigation = Windows::UI::Xaml::Input::KeyboardNavigationMode::Cycle;

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
				Key^ key = dynamic_cast<Key^>(ctrl->Content);
				if (key)
				{
					key->parent = ctrl;
					_keys.insert(std::make_pair(key, false));
				}
				
				// позже для оптимизации можно будет часть клавиш держать в одном массиве, часть в другом
				// сейчас ещё не до конца понятно, какие клавиши во что будем превращать

				// для тача
				ctrl->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::onKeyboardControlEntered);
				ctrl->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::OnPointerExited);

				// для мыши	
				ctrl->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::onKeyboardControlPressed);
				ctrl->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::OnPointerReleased);

				ctrl->KeyDown += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &BaseKeyboard::OnKeyDown);
				ctrl->KeyUp += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &BaseKeyboard::OnKeyUp);
			}
		}
	}

	// специально проверяем на тот случай если курсор убегает за пределы клавиш и мы не знаем, отпустили его или нет
	//grid->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::OnPointerReleased);
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

inline SketchMusic::View::Key^ GetKey(std::multimap<Key^, bool> keys, KeyType type, int val = 0)
{
	for (auto&& iter : keys)
	{
		if (iter.first->originalType == type && iter.first->originalValue == val && !iter.second) return iter.first;
	}
	return nullptr;
}

void SketchMusic::View::BaseKeyboard::OnKeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	if (e->KeyStatus.WasKeyDown) return;

	using Windows::System::VirtualKey;
	SketchMusic::View::Key^ smkey = nullptr;

	switch (e->Key)
	{
	case VirtualKey::Left: smkey = GetKey(_keys, KeyType::move, -1); break;
	case VirtualKey::Right: smkey = GetKey(_keys, KeyType::move, 1);break;
	case VirtualKey::Back: smkey = GetKey(_keys, KeyType::deleteSym);break;
	case VirtualKey::Space: smkey = GetKey(_keys, KeyType::play);break;
		// альтернатива для обобщённых нот?
	case VirtualKey::Number1: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::Number2: smkey = GetKey(_keys, KeyType::note, 12); break;
	case VirtualKey::Number3: smkey = GetKey(_keys, KeyType::note, 13); break;
	case VirtualKey::Number4: smkey = GetKey(_keys, KeyType::note, 14); break;
	case VirtualKey::Number5: smkey = GetKey(_keys, KeyType::note, 15); break;
	case VirtualKey::Number6: smkey = GetKey(_keys, KeyType::note, 16); break;
	case VirtualKey::Number7: smkey = GetKey(_keys, KeyType::note, 17); break;
	case VirtualKey::Number8: smkey = GetKey(_keys, KeyType::note, 18); break;
	case VirtualKey::Number9: smkey = GetKey(_keys, KeyType::note, 19); break;
	case VirtualKey::Number0: smkey = GetKey(_keys, KeyType::note, 20); break;
	case VirtualKey::Q: 
		smkey = GetKey(_keys, KeyType::note, 8); 
		break;
	case VirtualKey::W: smkey = GetKey(_keys, KeyType::note, 9); break;
	case VirtualKey::E: 
		smkey = GetKey(_keys, KeyType::note, 10);
		break;
	case VirtualKey::R: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::T: smkey = GetKey(_keys, KeyType::note, 12); break;
	case VirtualKey::Y: smkey = GetKey(_keys, KeyType::note, 13); break;
	case VirtualKey::U: smkey = GetKey(_keys, KeyType::note, 14); break;
	case VirtualKey::I: smkey = GetKey(_keys, KeyType::note, 15); break;
	case VirtualKey::O: smkey = GetKey(_keys, KeyType::note, 16); break;
	case VirtualKey::P: smkey = GetKey(_keys, KeyType::note, 17); break;
	case VirtualKey::A: smkey = GetKey(_keys, KeyType::note, 3); break;
	case VirtualKey::S: smkey = GetKey(_keys, KeyType::note, 4); break;
	case VirtualKey::D: smkey = GetKey(_keys, KeyType::note, 5); break;
	case VirtualKey::F: smkey = GetKey(_keys, KeyType::note, 6); break;
	case VirtualKey::G: smkey = GetKey(_keys, KeyType::note, 7); break;
	case VirtualKey::H: smkey = GetKey(_keys, KeyType::note, 8); break;
	case VirtualKey::J: smkey = GetKey(_keys, KeyType::note, 9); break;
	case VirtualKey::K: smkey = GetKey(_keys, KeyType::note, 10); break;
	case VirtualKey::L: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::Z: smkey = GetKey(_keys, KeyType::note, 0); break;
	case VirtualKey::X: smkey = GetKey(_keys, KeyType::note, 1); break;
	case VirtualKey::C: smkey = GetKey(_keys, KeyType::note, 2); break;
	case VirtualKey::V: smkey = GetKey(_keys, KeyType::note, 3); break;
	case VirtualKey::B: smkey = GetKey(_keys, KeyType::note, 4); break;
	case VirtualKey::N: smkey = GetKey(_keys, KeyType::note, 5); break;
	case VirtualKey::M: smkey = GetKey(_keys, KeyType::note, 6); break;
	case VirtualKey::Subtract: smkey = GetKey(_keys, KeyType::octave, 1); break;
	case VirtualKey::Add: smkey = GetKey(_keys, KeyType::octave, -1); break;
	case VirtualKey::Control: smkey = GetKey(_keys, KeyType::control, 0); break;
	case VirtualKey::Enter: smkey = GetKey(_keys, KeyType::enter, 0); break;
	case VirtualKey::Shift: smkey = GetKey(_keys, KeyType::record, 0); break;
	case VirtualKey::CapitalLock: smkey = GetKey(_keys, KeyType::record, 0); break; // капс лок не будем обрабатывать в релизе ? оО
	}
	OnPushKey(smkey);
}

void SketchMusic::View::BaseKeyboard::OnKeyUp(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	using Windows::System::VirtualKey;
	SketchMusic::View::Key^ smkey = nullptr;

	switch (e->Key)
	{
	case VirtualKey::Left: smkey = GetKey(_keys, KeyType::move, -1); break;
	case VirtualKey::Right: smkey = GetKey(_keys, KeyType::move, 1);break;
	case VirtualKey::Back: smkey = GetKey(_keys, KeyType::deleteSym);break;
	case VirtualKey::Space: smkey = GetKey(_keys, KeyType::play);break;
		// альтернатива для обобщённых нот?
	case VirtualKey::Number1: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::Number2: smkey = GetKey(_keys, KeyType::note, 12); break;
	case VirtualKey::Number3: smkey = GetKey(_keys, KeyType::note, 13); break;
	case VirtualKey::Number4: smkey = GetKey(_keys, KeyType::note, 14); break;
	case VirtualKey::Number5: smkey = GetKey(_keys, KeyType::note, 15); break;
	case VirtualKey::Number6: smkey = GetKey(_keys, KeyType::note, 16); break;
	case VirtualKey::Number7: smkey = GetKey(_keys, KeyType::note, 17); break;
	case VirtualKey::Number8: smkey = GetKey(_keys, KeyType::note, 18); break;
	case VirtualKey::Number9: smkey = GetKey(_keys, KeyType::note, 19); break;
	case VirtualKey::Number0: smkey = GetKey(_keys, KeyType::note, 20); break;
	case VirtualKey::Q:
		smkey = GetKey(_keys, KeyType::note, 8);
		break;
	case VirtualKey::W: smkey = GetKey(_keys, KeyType::note, 9); break;
	case VirtualKey::E:
		smkey = GetKey(_keys, KeyType::note, 10);
		break;
	case VirtualKey::R: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::T: smkey = GetKey(_keys, KeyType::note, 12); break;
	case VirtualKey::Y: smkey = GetKey(_keys, KeyType::note, 13); break;
	case VirtualKey::U: smkey = GetKey(_keys, KeyType::note, 14); break;
	case VirtualKey::I: smkey = GetKey(_keys, KeyType::note, 15); break;
	case VirtualKey::O: smkey = GetKey(_keys, KeyType::note, 16); break;
	case VirtualKey::P: smkey = GetKey(_keys, KeyType::note, 17); break;
	case VirtualKey::A: smkey = GetKey(_keys, KeyType::note, 3); break;
	case VirtualKey::S: smkey = GetKey(_keys, KeyType::note, 4); break;
	case VirtualKey::D: smkey = GetKey(_keys, KeyType::note, 5); break;
	case VirtualKey::F: smkey = GetKey(_keys, KeyType::note, 6); break;
	case VirtualKey::G: smkey = GetKey(_keys, KeyType::note, 7); break;
	case VirtualKey::H: smkey = GetKey(_keys, KeyType::note, 8); break;
	case VirtualKey::J: smkey = GetKey(_keys, KeyType::note, 9); break;
	case VirtualKey::K: smkey = GetKey(_keys, KeyType::note, 10); break;
	case VirtualKey::L: smkey = GetKey(_keys, KeyType::note, 11); break;
	case VirtualKey::Z: smkey = GetKey(_keys, KeyType::note, 0); break;
	case VirtualKey::X: smkey = GetKey(_keys, KeyType::note, 1); break;
	case VirtualKey::C: smkey = GetKey(_keys, KeyType::note, 2); break;
	case VirtualKey::V: smkey = GetKey(_keys, KeyType::note, 3); break;
	case VirtualKey::B: smkey = GetKey(_keys, KeyType::note, 4); break;
	case VirtualKey::N: smkey = GetKey(_keys, KeyType::note, 5); break;
	case VirtualKey::M: smkey = GetKey(_keys, KeyType::note, 6); break;
	case VirtualKey::Subtract: smkey = GetKey(_keys, KeyType::octave, 1); break;
	case VirtualKey::Add: smkey = GetKey(_keys, KeyType::octave, -1); break;
	case VirtualKey::Control: smkey = GetKey(_keys, KeyType::control, 0); break;
	case VirtualKey::Enter: smkey = GetKey(_keys, KeyType::enter, 0); break;
	case VirtualKey::Shift: smkey = GetKey(_keys, KeyType::record, 0); break;
	case VirtualKey::CapitalLock: smkey = GetKey(_keys, KeyType::record, 0); break; // капс лок не будем обрабатывать в релизе ? оО
	}
	OnReleaseKey(smkey);
}

void SketchMusic::View::BaseKeyboard::OnPointerExited(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	// отпускаем клавишу, если это тач или если мышь нажата и не отпускалась
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen)
		//&& (!mousePressed)
		)
		return;

	ReleaseKey(sender);
}


void SketchMusic::View::BaseKeyboard::ReleaseKey(Object^ sender)
{
	Windows::UI::Xaml::Controls::ContentControl^ ctrl =
		dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(static_cast<Object^>(sender));

	if (ctrl == nullptr) return;

	Key^ key = dynamic_cast<Key^>(ctrl->Content);
	if (key == nullptr) return;

	OnReleaseKey(key);
}

void SketchMusic::View::BaseKeyboard::OnReleaseKey(Key ^ key)
{
	auto ctrl = ((ContentControl^)key->parent);
	ctrl->Background = nullptr;

	KeyboardEventArgs^ args = ref new KeyboardEventArgs(key, this->pressedKeys);

	if (key)
	{
		switch (key->type)
		{
		case KeyType::tempo:
		{
			auto flyout = Windows::UI::Xaml::Controls::Flyout::GetAttachedFlyout(ctrl);
			if (flyout)
				flyout->ShowAt(ctrl);
			break;
		}
		case KeyType::quantization:
		{
			auto flyout = Windows::UI::Xaml::Controls::Flyout::GetAttachedFlyout(ctrl);
			if (flyout)
				flyout->ShowAt(ctrl);
			break;
		}
		case KeyType::hide:
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
		case KeyType::layout:
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


	pressedKeys--;

	if (pressedKeys <= 0)
	{
		pressedKeys = 0;	// на всякий случай принудительно

							// уведомляем всех о том, что всё норм
		this->currentState->state = KeyboardStateEnum::normal;
		KeyboardStateChanged(this, currentState);
	}
}


void SketchMusic::View::BaseKeyboard::OnClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
	auto key = ref new Key(KeyType::tempo, TempoSlider->Value);
	auto keyArgs = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, keyArgs);
	
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

		auto key = ref new Key(KeyType::quantization, div);
		auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
		KeyPressed(this, args);

		QuantizeFlyout->Hide();
	}
}

// Для задания начальных значений
void SketchMusic::View::BaseKeyboard::SetKey(KeyType type, int value)
{
	switch (type)
	{
	case KeyType::metronome:
		metroNeed->IsChecked = value ? true : false;
		break;
	case KeyType::precount:
		precountNeed->IsChecked = value ? true : false;
		break;
	case KeyType::playGeneric:
		gnoteNeed->IsChecked = value ? true : false;
		break;
	}
}


void SketchMusic::View::BaseKeyboard::quantizeNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	quantizeNeed->IsChecked = !quantizeNeed->IsChecked;
	auto key = ref new Key(KeyType::quantization, 0);	// 0 - опасно, но хочу попробовать
	auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, args);
}

void SketchMusic::View::BaseKeyboard::MenuFlyoutItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto el = dynamic_cast<FrameworkElement^>(e->OriginalSource);
	if (el)
	{
		int type = std::stoi(((String^)el->Tag)->Data());
		_layout = static_cast<KeyboardType>(type);
		
		switch (_layout)
		{
		case KeyboardType::Basic:
		case KeyboardType::Generic:
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
			break;
		default: break;
		}
		
	}

	LayoutFlyout->Hide();
}


void SketchMusic::View::BaseKeyboard::gnoteNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto key = ref new Key(KeyType::playGeneric, (int)(gnoteNeed->IsChecked->Value));
	auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, args);
}


void SketchMusic::View::BaseKeyboard::metroNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto key = ref new Key(KeyType::metronome, (int)(metroNeed->IsChecked->Value));
	auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, args);
}


void SketchMusic::View::BaseKeyboard::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TempoSlider->Value += 1;
}


void SketchMusic::View::BaseKeyboard::Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TempoSlider->Value -= 1;
}


void SketchMusic::View::BaseKeyboard::precountNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto key = ref new Key(KeyType::precount, 4*(int)(precountNeed->IsChecked->Value));
	auto args = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, args);
}


void SketchMusic::View::BaseKeyboard::TempoSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	BpmTxt->Text = L"BPM = " + TempoSlider->Value;
}
