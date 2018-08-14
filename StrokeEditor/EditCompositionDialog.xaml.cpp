//
// EditCompositionDialog.xaml.cpp
// Реализация класса EditCompositionDialog
//

#include "pch.h"
#include <string>

#include "EditCompositionDialog.xaml.h"

using namespace StrokeEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Документацию по шаблону элемента диалогового окна содержимого см. в разделе http://go.microsoft.com/fwlink/?LinkId=234238

StrokeEditor::EditCompositionDialog::EditCompositionDialog()
{
	InitializeComponent();
}

void StrokeEditor::EditCompositionDialog::ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
	auto comp = dynamic_cast<SketchMusic::Composition^>(this->DataContext);
	if (comp)
	{
		//comp->Header->FileName = CompositionFileName->Text;
		comp->Header->Name = CompositionName->Text;
		comp->Data->BPM = std::stof(CompositionBPM->Text->Data());
		comp->Header->Description = CompositionDescription->Text;
	}
}

void StrokeEditor::EditCompositionDialog::ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
}


void StrokeEditor::EditCompositionDialog::CompositionBPM_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
{
	if (sender->SelectionStart != 0)
	{
		std::wstring lastChar = sender->Text->Data();
		int pos = sender->SelectionStart - 1;
		lastChar = lastChar.substr(pos, 1);
		if (!iswdigit(lastChar.at(0)))
		{
			std::wstring wstr = sender->Text->Data();
			wstr = wstr.substr(0, pos);
			sender->Text = ref new Platform::String(wstr.data());
			sender->SelectionStart = pos;
		}
	}
}


void StrokeEditor::EditCompositionDialog::ContentDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args)
{
	auto comp = dynamic_cast<SketchMusic::Composition^>(this->DataContext);
	if (comp)
	{
		CompositionFileName->Text = comp->Header->FileName;
		CompositionName->Text = comp->Header->Name;
		CompositionBPM->Text = comp->Data->BPM.ToString();
		CompositionDescription->Text = comp->Header->Description;
	}
}
