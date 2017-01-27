//
// ManageInstrumentsDialog.xaml.h
// Объявление класса ManageInstrumentsDialog
//

#pragma once

#include "ManageInstrumentsDialog.g.h"

namespace StrokeEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ManageInstrumentsDialog sealed
	{
	public:
		property Windows::Foundation::Collections::IObservableVector<SketchMusic::Instrument^>^ Instruments;
		property Windows::Foundation::Collections::IObservableVector<SketchMusic::SFReader::SFPreset^>^ Presets;
		property Windows::Foundation::Collections::IObservableVector<SketchMusic::Instrument^>^ Texts;

		property SketchMusic::Instrument^ Selected;

		ManageInstrumentsDialog();
	private:
		void ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args);
		void ContentDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args);
		void InstrumentList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		//SketchMusic::CompositionData^ test;
		//SketchMusic::Text^ testText;
	};
}
