﻿//
// BaseKeyboard.xaml.h
// Объявление класса BaseKeyboard
//

#pragma once

#include "../SketchMusic.h"
#include "Converters.h"
#include "Key.h"
#include "SketchMusic\view\GenericKeyboard.g.h"
#include <string>


using namespace Platform;
using namespace SketchMusic;
using namespace SketchMusic::View;


namespace SketchMusic
{
	namespace View
	{
		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class BaseKeyboard sealed // : public SketchMusic::View::IKeyboard //: public Windows::UI::Xaml::Controls::Control
		{
		private:
			bool input;
			bool tempoPressed;
			//std::vector<SketchMusic::View::Key^> inputBuffer;
			//int octaveModificator;

			void PushKey(Object^ object);
			void ReleaseKey(Object^ object);
			void OnKeyboardPressed(SketchMusic::View::Key^ key);
			void OnControlPressed(SketchMusic::View::Key^ key);
			void OnNormalState(SketchMusic::View::Key^ key);
			void OnPlayStopState(SketchMusic::View::Key^ key);

			SketchMusic::View::KeyboardState^ currentState;	// состояние клавиатуры. Изменяется при нажатии всяких контрол, шифт и так далее

			int pressedKeys;	// суммарное количество нажатых клавиш


			std::multimap<SketchMusic::View::Key^, bool> _keys;	// сохраняем ссылки на клавиши, чтобы уведомлять их об изменении состояния
																// вторым параметром отмечаем, что клавиша в данный момент нажата

			concurrency::cancellation_token_source releaseToken;

			Windows::UI::Xaml::ResourceDictionary^ _dict;

			SketchMusic::View::KeyboardType _layout;

		public:
			BaseKeyboard();

			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyPressed;	// нажатие на одну клавишу
			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyReleased;	// отпускание одной клавиши
			event EventHandler<Windows::Foundation::Collections::IVector<SketchMusic::View::Key^>^>^ KeyboardReleased; // "отпускание" всех клавиш
			event EventHandler<SketchMusic::View::KeyboardState^>^ KeyboardStateChanged;

			void OnKeyboardStateChanged(Object^ object, SketchMusic::View::KeyboardState ^ state);
			void InitializePage();

			// для тача
			void onKeyboardControlEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
			void OnPointerExited(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);

			// для мыши
			void onKeyboardControlPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
			void OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			void OnClosed(Platform::Object ^sender, Platform::Object ^args);
			void OnOpened(Platform::Object ^sender, Platform::Object ^args);
			void OnClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
			void OnQuantizeClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
		private:
			void quantizeNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void QuantizeFlyout_Closed(Platform::Object^ sender, Platform::Object^ e);
			void MenuFlyoutItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		};
	}
}