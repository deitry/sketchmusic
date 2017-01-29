//
// BaseKeyboard.xaml.h
// Объявление класса BaseKeyboard
//

#pragma once

#include "../SketchMusic.h"
#include "Converters.h"
#include "Key.h"
#include "SketchMusic\view\BaseKeyboard.g.h"
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
			bool ctrlPressed;

			bool isTempoFlyoutOpened;
			bool isQuantizeFlyoutOpened;
			bool isLayoutFlyoutOpened;

			//std::vector<SketchMusic::View::Key^> inputBuffer;
			//int octaveModificator;

			void PushKey(Object^ object);
			void OnPushKey(Key^ key);
			void ReleaseKey(Object^ object);
			void OnReleaseKey(Key^ key);
			void OnKeyboardPressed(SketchMusic::View::Key^ key);
			void OnControlPressed(SketchMusic::View::Key^ key);
			void OnNormalState(SketchMusic::View::Key^ key);
			void OnPlayStopState(SketchMusic::View::Key^ key);

			SketchMusic::View::KeyboardState^ currentState;	// состояние клавиатуры. Изменяется при нажатии всяких контрол, шифт и так далее

			int pressedKeys;	// суммарное количество нажатых клавиш
			

			//Windows::UI::Input::GestureRecognizer^ gr;
			//void gestureRecognizer_Holding(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::HoldingEventArgs^ args);


			std::vector<std::pair<Windows::UI::Xaml::Controls::ContentControl^, bool>> _keys;	// сохраняем ссылки на клавиши, чтобы уведомлять их об изменении состояния
																// вторым параметром отмечаем, что клавиша в данный момент нажата
			
			std::pair<Windows::UI::Xaml::Controls::ContentControl^, bool> 
				GetControl(std::vector<std::pair<Windows::UI::Xaml::Controls::ContentControl^, bool>> keys, Platform::String^ tag);

			concurrency::cancellation_token_source releaseToken;
			concurrency::cancellation_token_source HoldingTokenSource;

			Windows::Foundation::EventRegistrationToken KeyDownToken;
			Windows::Foundation::EventRegistrationToken KeyUpToken;

			Windows::UI::Xaml::ResourceDictionary^ _dict;

			SketchMusic::View::KeyboardType _layout;
		protected:
			virtual void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e) override;
			virtual void OnUnloaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e) override;

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
			void OnPointerMoved(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			void OnHolding(Platform::Object ^sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs ^e);
			void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs ^e);
			void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs ^e);
			void OnClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
			void OnQuantizeClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);

			void SetKey(KeyType type, int value);
		private:
			void quantizeNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void MenuFlyoutItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void gnoteNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void metroNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void precountNeed_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void TempoSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
			void QuantizeFlyout_Opened(Platform::Object^ sender, Platform::Object^ e);
			void QuantizeFlyout_Closed(Platform::Object^ sender, Platform::Object^ e);
			void TempoFlyout_Opened(Platform::Object^ sender, Platform::Object^ e);
			void TempoFlyout_Closed(Platform::Object^ sender, Platform::Object^ e);
			void LayoutFlyout_Opened(Platform::Object^ sender, Platform::Object^ e);
			void LayoutFlyout_Closed(Platform::Object^ sender, Platform::Object^ e);
		};
	}
}
