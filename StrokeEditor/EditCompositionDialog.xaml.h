//
// EditCompositionDialog.xaml.h
// Объявление класса EditCompositionDialog
//

#pragma once

#include "EditCompositionDialog.g.h"

namespace StrokeEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class EditCompositionDialog sealed
	{
	public:
		EditCompositionDialog();
	private:
		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void CompositionBPM_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args);
		void ContentDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args);
	};
}
