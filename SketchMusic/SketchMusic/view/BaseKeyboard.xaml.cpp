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
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

namespace
{
	// сдвигать клавиатуру по полутонам вместо октавы
	bool alternativeControl = false;
	bool currentCtrlValue = false;

	using namespace SketchMusic::View;

	std::map<KeyboardType, KeyType> keyboardLayoutKeyType =
	{
		{ KeyboardType::Basic, KeyType::note },
		{ KeyboardType::Classic, KeyType::note },
		{ KeyboardType::Generic, KeyType::genericNote },
		{ KeyboardType::Relative, KeyType::relativeNote },
		{ KeyboardType::Harmony, KeyType::harmony },
		{ KeyboardType::Scale, KeyType::scale },
	};
}

SketchMusic::View::BaseKeyboard::BaseKeyboard()
{
	this->Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::BaseKeyboard::OnLoaded);
	this->Unloaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::BaseKeyboard::OnUnloaded);

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
	case KeyType::scale:
	case KeyType::harmony:
	case KeyType::localHarmony:
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
	case KeyType::move:
		key->type = KeyType::beat;
		UpdateParent(key);
		break;
	case KeyType::beat:
		key->type = KeyType::move;
		UpdateParent(key);
		break;
	case KeyType::harmony:
		if (::alternativeControl && currentCtrlValue)
		{
			key->type = KeyType::localHarmony;
			UpdateParent(key);
		}
		break;
	case KeyType::localHarmony:
		if (!::alternativeControl && currentCtrlValue)
		{
			key->type = KeyType::harmony;
			UpdateParent(key);
		}
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
	case KeyType::harmony:
		key->value += ::alternativeControl ? 1 : 12;
		UpdateParent(key);
		break;	
	case KeyType::localHarmony:
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
	case KeyType::harmony:
		key->value -= ::alternativeControl ? 1 : 12;
		UpdateParent(key);
		break;
	case KeyType::localHarmony:
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

//void SketchMusic::View::BaseKeyboard::gestureRecognizer_Holding(GestureRecognizer ^ sender, HoldingEventArgs ^ args)
//{
//	if (args->HoldingState == HoldingState::Started)
//	{
//		concurrency::create_task([=]
//		{
//			while (true)
//			{
//				wait(100);
//				PushKey(sender);
//			}
//		}, HoldingTokenSource.get_token());
//	}
//	else
//	{
//		HoldingTokenSource.cancel();
//	}
//}

void SketchMusic::View::BaseKeyboard::OnKeyboardStateChanged(Object^ object, KeyboardState ^ state)
{
	// проходимся по всем кнопкам и меняем их состояние согласно их типу
	for (auto&& pkey : _keys)
	{
		auto ctrl = pkey.first;
		auto key = dynamic_cast<Key^>(ctrl->Content);
		switch (state->state)
		{
		case KeyboardStateEnum::control:
			OnControlPressed(key);
			break;
		case KeyboardStateEnum::pressed:
			OnKeyboardPressed(key);
			break;
		case KeyboardStateEnum::octaveInc:
			// берём originalType, потому что в процессе работы тип клавиш может поменяться
			if (key->originalType == ::keyboardLayoutKeyType.at(_layout))
				OnOctaveInc(key);
			break;
		case KeyboardStateEnum::octaveDec:
			// берём originalType, потому что в процессе работы тип клавиш может поменяться
			if (key->originalType == ::keyboardLayoutKeyType.at(_layout))
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
		ctrl->Background = (SolidColorBrush^)_dict->Lookup("pressedKeyBackground");
		Key^ key = dynamic_cast<Key^>(ctrl->Content);
		for (auto&& iter : _keys)
		{
			if (iter.first == ctrl)
			{
				iter.second = true;
				break;
			}
		}
		OnPushKey(key);

		if (key->type == KeyType::backspace || key->type == KeyType::move)
		{
			HoldingTokenSource = concurrency::cancellation_token_source();
			Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
				Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
			{
				auto cancelToken = HoldingTokenSource.get_token();
				concurrency::create_task([=]
				{
					concurrency::wait(500);
					int i = 0;
					while (true)
					{
						if (i < 10) wait(100);
						else if (i < 100) wait(30);
						else wait(10);

						if (cancelToken.is_canceled() || pressedKeys == 0)
							concurrency::cancel_current_task();

						OnPushKey(key);
						i++;
					}
				}, cancelToken);
			}));
		}
	}
}

void SketchMusic::View::BaseKeyboard::OnPushKey(Key ^ key)
{
	if (key == nullptr) return;
	KeyboardEventArgs^ args = ref new KeyboardEventArgs(key, this->pressedKeys);

	if (key)
	{
		switch (key->type)
		{
		case KeyType::note:
		case KeyType::genericNote:
		case KeyType::relativeNote:
		case KeyType::scale:
		case KeyType::harmony:
		case KeyType::localHarmony:
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
			if (!key->value) ::alternativeControl = !::alternativeControl; 
			currentCtrlValue = key->value;

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
		case KeyType::beat:
		case KeyType::space:
		case KeyType::backspace:
		default:
			KeyPressed(this, args);
			break;
		}
	}
}

void SketchMusic::View::BaseKeyboard::onKeyboardControlPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// не пропускаем тач
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	PushKey(sender);
	//mousePressed = true;
	auto el = dynamic_cast<Windows::UI::Xaml::FrameworkElement^>(sender);
	el->CapturePointer(e->Pointer);

	//auto ps = e->GetIntermediatePoints(nullptr);
	//if (ps && ps->Size > 0)
	//{
	//	gr->ProcessDownEvent(ps->GetAt(0));
	//	e->Handled = true;
	//}
}

void SketchMusic::View::BaseKeyboard::onKeyboardControlEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// пропускаем только нажатие тачем, не пропускаем мышку и стилус
	if ((e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen)
		)
		return;

	PushKey(sender);
}

void SketchMusic::View::BaseKeyboard::InitializePage()
{
	//this->TabNavigation = Windows::UI::Xaml::Input::KeyboardNavigationMode::Cycle;
	//gr = ref new GestureRecognizer;
	//gr->GestureSettings = Windows::UI::Input::GestureSettings::HoldWithMouse | Windows::UI::Input::GestureSettings::Hold;
	//gr->Holding += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Input::GestureRecognizer ^, Windows::UI::Input::HoldingEventArgs ^>
	//	(this, &SketchMusic::View::BaseKeyboard::gestureRecognizer_Holding);
	//gr->ShowGestureFeedback = true;

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
					_keys.push_back(std::make_pair(ctrl, false));
				}
				
				// позже для оптимизации можно будет часть клавиш держать в одном массиве, часть в другом
				// сейчас ещё не до конца понятно, какие клавиши во что будем превращать

				// для тача
				ctrl->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::onKeyboardControlEntered);
				ctrl->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::OnPointerExited);
				//if (key->type == KeyType::backspace || key->type == KeyType::move)
				//	ctrl->Holding += ref new Windows::UI::Xaml::Input::HoldingEventHandler(this, &BaseKeyboard::OnHolding);

				// для мыши	
				ctrl->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::onKeyboardControlPressed);
				ctrl->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &BaseKeyboard::OnPointerReleased);
			}
		}
	}
}

void SketchMusic::View::BaseKeyboard::OnLoaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	KeyDownToken =
		Window::Current->CoreWindow->KeyDown
		+= ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &SketchMusic::View::BaseKeyboard::OnKeyDown);
	KeyUpToken =
		Window::Current->CoreWindow->KeyUp
		+= ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &SketchMusic::View::BaseKeyboard::OnKeyUp);
}

void SketchMusic::View::BaseKeyboard::OnUnloaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	Window::Current->CoreWindow->KeyUp -= KeyUpToken;
	Window::Current->CoreWindow->KeyDown -= KeyDownToken;
}


void SketchMusic::View::BaseKeyboard::OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	//mousePressed = false;
	// TODO : отключать и тогда, если "отжали" мышку вне гридов и контролов

	// отпускаем клавишу, если это не тач
	if (e->Pointer->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	ReleaseKey(sender);

	//auto ps = e->GetIntermediatePoints(nullptr);
	//if (ps != nullptr && ps->Size > 0)
	//{
	//	gr->ProcessUpEvent(ps->GetAt(0));
	//	e->Handled = true;
	//	gr->CompleteGesture();
	//}
}

void SketchMusic::View::BaseKeyboard::OnPointerMoved(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e)
{
	//gr->ProcessMoveEvents(e->GetIntermediatePoints(nullptr));
	//e->Handled = true;
}

void SketchMusic::View::BaseKeyboard::OnHolding(Platform::Object ^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs ^ e)
{

}

std::pair<ContentControl^, bool> SketchMusic::View::BaseKeyboard::GetControl(std::vector<std::pair<ContentControl^, bool>> keys, String^ tag)
{
	for (auto&& ctrl : keys)
	{
		auto key = dynamic_cast<SketchMusic::View::Key^>(ctrl.first->Content);
		if (key == nullptr) continue;

		if (key->type == KeyType::note 
			|| key->type == KeyType::genericNote
			|| key->type == KeyType::scale
			|| key->type == KeyType::harmony
			|| key->type == KeyType::localHarmony
			|| key->type == KeyType::relativeNote)
		{
			// проверка на тип раскладки - только для нот
			auto el = dynamic_cast<FrameworkElement^>(ctrl.first->Parent);
			if (el == nullptr) continue;
			auto str = (String^)el->Tag;int type = std::stoi(str->Data());
			KeyboardType kType = static_cast<KeyboardType>(type);
			if (kType != _layout) continue;
		}

		if (((String^)ctrl.first->Tag) == tag)
		{
			return ctrl;
		}
	}
	return std::make_pair(nullptr,false);
}

void SketchMusic::View::BaseKeyboard::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs ^e)
{
	if (e->KeyStatus.WasKeyDown == true)
		return;

	using Windows::System::VirtualKey;
	std::pair<ContentControl^, bool> ctrl;

	bool isShift = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Shift) != Windows::UI::Core::CoreVirtualKeyStates::None);
	bool isCtrl = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Control) != Windows::UI::Core::CoreVirtualKeyStates::None);
	bool isAlt = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Menu) != Windows::UI::Core::CoreVirtualKeyStates::None);

	//if (isShift && isAlt)
	//{
	//	ctrl = GetControl(_keys, "layout");
	//	if (ctrl.first && !ctrl.second)
	//		PushKey(ctrl.first);
	//}

	switch (e->VirtualKey)
	{
	case VirtualKey::Left: ctrl = GetControl(_keys, "left"); break;
	case VirtualKey::Right: ctrl = GetControl(_keys, "right");break;
	case VirtualKey::Back: ctrl = GetControl(_keys, "backsp");break;
	case VirtualKey::Space: ctrl = GetControl(_keys, "play");break;
		// альтернатива для обобщённых нот?
	case VirtualKey::Number1: 
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q1, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_1"); break;
	case VirtualKey::Number2: 
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q2, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_2"); break;
	case VirtualKey::Number3: 
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q3, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_3"); break;
	case VirtualKey::Number4: 
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q4, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_4"); break;
	case VirtualKey::Number5:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q5, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_5"); break;
	case VirtualKey::Number6:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q1t, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_6"); break;
	case VirtualKey::Number7:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q2t, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_7"); break;
	case VirtualKey::Number8:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q3t, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_8"); break;
	case VirtualKey::Number9:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q4t, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_9"); break;
	case VirtualKey::Number0:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			OnQuantizeClick(q5t, nullptr); return;
		}
		else ctrl = GetControl(_keys, "1_10"); break;
	case VirtualKey::Q:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "quant");
		else ctrl = GetControl(_keys, "2_2"); 
		break;
	case VirtualKey::W: 
		if (ctrlPressed)
			ctrl = GetControl(_keys, "end");
		else ctrl = GetControl(_keys, "2_3"); break;
	case VirtualKey::E: 
		if (ctrlPressed)
			ctrl = GetControl(_keys, "eraser");
		else ctrl = GetControl(_keys, "2_4");
		break;
	case VirtualKey::R: ctrl = GetControl(_keys, "2_5"); break;
	case VirtualKey::T: 
		if (ctrlPressed)
			ctrl = GetControl(_keys, "tempo");
		else ctrl = GetControl(_keys, "2_6"); break;
	case VirtualKey::Y: if (!ctrlPressed) ctrl = GetControl(_keys, "2_7"); break;
	case VirtualKey::U: ctrl = GetControl(_keys, "2_8"); break;
	case VirtualKey::I: ctrl = GetControl(_keys, "2_9"); break;
	case VirtualKey::O: ctrl = GetControl(_keys, "2_10"); break;
	case VirtualKey::P: ctrl = GetControl(_keys, "2_11"); break;
	case (VirtualKey)219: ctrl = GetControl(_keys, "2_12"); break;
	case (VirtualKey)221: ctrl = GetControl(_keys, "2_13"); break;
	case VirtualKey::A: ctrl = GetControl(_keys, "3_2"); break;
	case VirtualKey::S: if (!ctrlPressed) ctrl = GetControl(_keys, "3_3"); break;
	case VirtualKey::D: ctrl = GetControl(_keys, "3_4"); break;
	case VirtualKey::F: ctrl = GetControl(_keys, "3_5"); break;
	case VirtualKey::G: ctrl = GetControl(_keys, "3_6"); break;
	case VirtualKey::H:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "hide");
		else ctrl = GetControl(_keys, "3_7"); break;
	case VirtualKey::J: ctrl = GetControl(_keys, "3_8"); break;
	case VirtualKey::K: ctrl = GetControl(_keys, "3_9"); break;
	case VirtualKey::L:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "cycle");
		else ctrl = GetControl(_keys, "3_10"); break;
	case (VirtualKey) 186: ctrl = GetControl(_keys, "3_11"); break;
	case VirtualKey::Z: if (!ctrlPressed) ctrl = GetControl(_keys, "4_3"); break;
	case VirtualKey::X: ctrl = GetControl(_keys, "4_4"); break;
	case VirtualKey::C: ctrl = GetControl(_keys, "4_5"); break;
	case VirtualKey::V: ctrl = GetControl(_keys, "4_6"); break;
	case VirtualKey::B: ctrl = GetControl(_keys, "4_7"); break;
	case VirtualKey::N: ctrl = GetControl(_keys, "4_8"); break;
	case VirtualKey::M: ctrl = GetControl(_keys, "4_9"); break;
	case (VirtualKey)188: ctrl = GetControl(_keys, "4_10"); break;
	case (VirtualKey)190: ctrl = GetControl(_keys, "4_11"); break;
	case (VirtualKey)191: ctrl = GetControl(_keys, "4_12"); break;

	case VirtualKey::Down:
	case (VirtualKey)189:
	case VirtualKey::Subtract: ctrl = GetControl(_keys, "-"); break;
	
	case VirtualKey::Up:
	case (VirtualKey)187:
	case VirtualKey::Add: ctrl = GetControl(_keys, "+"); break;
	case VirtualKey::Control: ctrlPressed = true; ctrl = GetControl(_keys, "ctrl"); break;
	case VirtualKey::Enter:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "end");
		else ctrl = GetControl(_keys, "enter"); break;
	case VirtualKey::Shift: ctrl = GetControl(_keys, "rec"); break;	
	case VirtualKey::CapitalLock: ctrl = GetControl(_keys, "rec"); break;
	}

	if ((isTempoFlyoutOpened || isLayoutFlyoutOpened || isQuantizeFlyoutOpened))
	{
		if ((e->VirtualKey >= VirtualKey::Number0) && (e->VirtualKey <= VirtualKey::Number9) && (ctrl.first && !ctrl.second))
			PushKey(ctrl.first);
	}
	else if (ctrl.first && !ctrl.second)
		PushKey(ctrl.first);
}

void SketchMusic::View::BaseKeyboard::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs ^e)
{
	using Windows::System::VirtualKey;
	std::pair<ContentControl^, bool> ctrl;

	bool isShift = sender->GetForCurrentThread()->GetKeyState(VirtualKey::Shift) != Windows::UI::Core::CoreVirtualKeyStates::None;
	bool isCtrl = sender->GetForCurrentThread()->GetKeyState(VirtualKey::Control) != Windows::UI::Core::CoreVirtualKeyStates::None;
	bool isAlt = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Menu) != Windows::UI::Core::CoreVirtualKeyStates::None);

	//if (isShift && isAlt)
	//{
	//	ctrl = GetControl(_keys, "layout");
	//	if (ctrl.first && !ctrl.second)
	//		ReleaseKey(ctrl.first);
	//}

	switch (e->VirtualKey)
	{
	case VirtualKey::Left: ctrl = GetControl(_keys, "left"); break;
	case VirtualKey::Right: ctrl = GetControl(_keys, "right");break;
	case VirtualKey::Back: ctrl = GetControl(_keys, "backsp");break;
	case VirtualKey::Space: ctrl = GetControl(_keys, "play");break;
		// альтернатива для обобщённых нот?
	case VirtualKey::Number1:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_1"); break;
	case VirtualKey::Number2:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_2"); break;
	case VirtualKey::Number3:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_3"); break;
	case VirtualKey::Number4:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_4"); break;
	case VirtualKey::Number5:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_5"); break;
	case VirtualKey::Number6:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_6"); break;
	case VirtualKey::Number7:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_7"); break;
	case VirtualKey::Number8:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_8"); break;
	case VirtualKey::Number9:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_9"); break;
	case VirtualKey::Number0:
		if (ctrlPressed && isQuantizeFlyoutOpened && 1)
		{
			return;
		}
		else ctrl = GetControl(_keys, "1_10"); break;
	case VirtualKey::Q:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "quant");
		else ctrl = GetControl(_keys, "2_2");
		break;
	case VirtualKey::W: 
		if (ctrlPressed)
			ctrl = GetControl(_keys, "end");
		else ctrl = GetControl(_keys, "2_3"); break;
	case VirtualKey::E:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "eraser");
		else ctrl = GetControl(_keys, "2_4");
		break;
	case VirtualKey::R: ctrl = GetControl(_keys, "2_5"); break;
	case VirtualKey::T:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "tempo");
		else ctrl = GetControl(_keys, "2_6"); break;
	case VirtualKey::Y: ctrl = GetControl(_keys, "2_7"); break;
	case VirtualKey::U: ctrl = GetControl(_keys, "2_8"); break;
	case VirtualKey::I: ctrl = GetControl(_keys, "2_9"); break;
	case VirtualKey::O: ctrl = GetControl(_keys, "2_10"); break;
	case VirtualKey::P: ctrl = GetControl(_keys, "2_11"); break;
	case (VirtualKey)219: ctrl = GetControl(_keys, "2_12"); break;
	case (VirtualKey)221: ctrl = GetControl(_keys, "2_13"); break;
	case VirtualKey::A: ctrl = GetControl(_keys, "3_2"); break;
	case VirtualKey::S: ctrl = GetControl(_keys, "3_3"); break;
	case VirtualKey::D: ctrl = GetControl(_keys, "3_4"); break;
	case VirtualKey::F: ctrl = GetControl(_keys, "3_5"); break;
	case VirtualKey::G: ctrl = GetControl(_keys, "3_6"); break;
	case VirtualKey::H:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "hide");
		else ctrl = GetControl(_keys, "3_7"); break;
	case VirtualKey::J: ctrl = GetControl(_keys, "3_8"); break;
	case VirtualKey::K: ctrl = GetControl(_keys, "3_9"); break;
	case VirtualKey::L:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "cycle");
		else ctrl = GetControl(_keys, "3_10"); break;
	case (VirtualKey)186: ctrl = GetControl(_keys, "3_11"); break;
	case VirtualKey::Z: ctrl = GetControl(_keys, "4_3"); break;
	case VirtualKey::X: ctrl = GetControl(_keys, "4_4"); break;
	case VirtualKey::C: ctrl = GetControl(_keys, "4_5"); break;
	case VirtualKey::V: ctrl = GetControl(_keys, "4_6"); break;
	case VirtualKey::B: ctrl = GetControl(_keys, "4_7"); break;
	case VirtualKey::N: ctrl = GetControl(_keys, "4_8"); break;
	case VirtualKey::M: ctrl = GetControl(_keys, "4_9"); break;
	case (VirtualKey)188: ctrl = GetControl(_keys, "4_10"); break;
	case (VirtualKey)190: ctrl = GetControl(_keys, "4_11"); break;
	case (VirtualKey)191: ctrl = GetControl(_keys, "4_12"); break;

	case VirtualKey::Down:
	case (VirtualKey)189:
	case VirtualKey::Subtract: ctrl = GetControl(_keys, "-"); break;
	case (VirtualKey)187:
	case VirtualKey::Up:
	case VirtualKey::Add: ctrl = GetControl(_keys, "+"); break;
	case VirtualKey::Control: ctrlPressed = false; ctrl = GetControl(_keys, "ctrl"); if (ctrl.first) { ReleaseKey(ctrl.first); PushKey(ctrl.first); } break;
	case VirtualKey::Enter:
		if (ctrlPressed)
			ctrl = GetControl(_keys, "end");
		else ctrl = GetControl(_keys, "enter"); break;
	case VirtualKey::Shift: ctrl = GetControl(_keys, "rec"); if (ctrl.first) { ReleaseKey(ctrl.first); PushKey(ctrl.first); } break;
	case VirtualKey::CapitalLock: ctrl = GetControl(_keys, "rec"); break;
	}
	if (ctrl.first)
		ReleaseKey(ctrl.first);
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
	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	ctrl->Background = nullptr;

	if (ctrl == nullptr) return;
	for (auto&& iter : _keys)
	{
		if (iter.first == ctrl)
		{
			iter.second = false;
			break;
		}
	}

	Key^ key = dynamic_cast<Key^>(ctrl->Content);
	if (key == nullptr) return;

	HoldingTokenSource.cancel();
	OnReleaseKey(key);
}

void SketchMusic::View::BaseKeyboard::OnReleaseKey(Key ^ key)
{
	if (key == nullptr) return;

	auto ctrl = ((ContentControl^)key->parent);

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

					row->Visibility = (key->value == 0 && tagType == _layout) 
										? Windows::UI::Xaml::Visibility::Visible
										: Windows::UI::Xaml::Visibility::Collapsed;
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
		case KeyType::scale:
		case KeyType::harmony:
		case KeyType::localHarmony:
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


void SketchMusic::View::BaseKeyboard::OnClick(Platform::Object ^sender, 
											  Windows::UI::Xaml::RoutedEventArgs ^e)
{
	auto key = ref new Key(KeyType::tempo, static_cast<int>(TempoSlider->Value));
	auto keyArgs = ref new KeyboardEventArgs(key, this->pressedKeys);
	KeyPressed(this, keyArgs);
	
	TempoFlyout->Hide();
}

void SketchMusic::View::BaseKeyboard::OnQuantizeClick(Platform::Object ^ sender, 
													  Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	// 
	auto bt = dynamic_cast<Button^>(sender);//e->OriginalSource);
	if (bt)
	{
		quantizeNeed->IsChecked = true;

		// на сколько надо поделить
		int div = std::stoi(((String^)bt->DataContext)->Data());

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
		case KeyboardType::Classic:
		case KeyboardType::Scale:
		case KeyboardType::Harmony:
		case KeyboardType::Relative:
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


void SketchMusic::View::BaseKeyboard::QuantizeFlyout_Opened(Platform::Object^ sender, Platform::Object^ e)
{
	isQuantizeFlyoutOpened = true;
}


void SketchMusic::View::BaseKeyboard::QuantizeFlyout_Closed(Platform::Object^ sender, Platform::Object^ e)
{
	isQuantizeFlyoutOpened = false;
}


void SketchMusic::View::BaseKeyboard::TempoFlyout_Opened(Platform::Object^ sender, Platform::Object^ e)
{
	isTempoFlyoutOpened = true;
}


void SketchMusic::View::BaseKeyboard::TempoFlyout_Closed(Platform::Object^ sender, Platform::Object^ e)
{
	isTempoFlyoutOpened = false;
}


void SketchMusic::View::BaseKeyboard::LayoutFlyout_Opened(Platform::Object^ sender, Platform::Object^ e)
{
	isLayoutFlyoutOpened = true;
}


void SketchMusic::View::BaseKeyboard::LayoutFlyout_Closed(Platform::Object^ sender, Platform::Object^ e)
{
	isLayoutFlyoutOpened = false;
}
