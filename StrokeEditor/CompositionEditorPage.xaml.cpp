//
// CompositionEditorPage.xaml.cpp
// Реализация класса CompositionEditorPage
//

#include <string>
#include "pch.h"
#include "CompositionEditorPage.xaml.h"
#include "MainMenuPage.xaml.h"

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
using namespace Windows::UI::Xaml::Interop;
using namespace SketchMusic;


// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

CompositionEditorPage::CompositionEditorPage()
{
	InitializeComponent();

	CompositionPartList->ItemsSource = CompositionView->Parts;
}


void StrokeEditor::CompositionEditorPage::HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// идём на вкладку с обзором данных с пустыми руками
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid), nullptr);
}


void StrokeEditor::CompositionEditorPage::DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width - CompositionView->Margin.Left - CompositionView->Margin.Right;
	CompositionView->Width = width;
	CompositionView->UpdateSize();

	CompositionPartList->Width = width;
}

void StrokeEditor::CompositionEditorPage::CompositionPartList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::CompositionPartList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	DeletePartBtn->IsEnabled = CompositionPartList->SelectedItem ? true : false;
	EditPartBtn->IsEnabled = CompositionPartList->SelectedItem ? true : false;
	CompositionView->SelectedItem = (PartDefinition^)CompositionPartList->SelectedItem;
}


void StrokeEditor::CompositionEditorPage::DeletePartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	unsigned int ind;
	auto selected = CompositionPartList->SelectedItem;
	if (CompositionView->Parts->IndexOf((PartDefinition^)selected, &ind))
	{
		CompositionView->SelectedItem = nullptr;
		CompositionView->Parts->RemoveAt(ind);
	}
}


void StrokeEditor::CompositionEditorPage::AddPartButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	AddPartFlyout->Hide();
	
	PartDefinition^ part = ref new PartDefinition;
	part->length = std::stoi(PartLength->Text->Data());
	part->original = ref new SNewPart;
	part->original->category = PartCatTxt->Text;
	part->original->dynamic = (DynamicCategory)PartCatComboBox->SelectedIndex;

	CompositionView->Parts->Append(part);
}


void StrokeEditor::CompositionEditorPage::EditPartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::PartLength_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
{
	if (sender->SelectionStart != 0)
	{
		std::wstring lastChar = sender->Text->Data();
		int pos = sender->SelectionStart - 1;
		lastChar = lastChar.substr(pos,1);
		if (!iswdigit(lastChar.at(0)))
		{
			std::wstring wstr = sender->Text->Data();
			wstr = wstr.substr(0,pos);
			sender->Text = ref new Platform::String(wstr.data());
			sender->SelectionStart = pos;
		}
	}
}
