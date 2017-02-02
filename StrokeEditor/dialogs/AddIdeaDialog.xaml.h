//
// AddIdeaDialog.xaml.h
// Объявление класса AddIdeaDialog
//

#pragma once

#include "dialogs\AddIdeaDialog.g.h"

namespace StrokeEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AddIdeaDialog sealed
	{
	public:
		AddIdeaDialog();

		property SketchMusic::Idea^ NewIdea;

	private:
		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void CategoryGrid_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
