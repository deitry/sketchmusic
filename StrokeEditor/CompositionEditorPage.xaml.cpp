//
// CompositionEditorPage.xaml.cpp
// Реализация класса CompositionEditorPage
//

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





