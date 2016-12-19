//
// MainMenuPage.xaml.cpp
// Реализация класса MainMenuPage
//

#include "pch.h"
#include "KeyboardPage.xaml.h"
#include "MainMenuPage.xaml.h"
#include "AboutPage.xaml.h"
#include "LibraryOverviewPage.xaml.h"


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

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

MainMenuPage::MainMenuPage()
{
	InitializeComponent();
}


void StrokeEditor::MainMenuPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::MainMenuPage::OpenEditorBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::KeyboardPage::typeid));
}


void StrokeEditor::MainMenuPage::OpenLibraryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryOverviewPage::typeid));
}


void StrokeEditor::MainMenuPage::AboutBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::AboutPage::typeid));
}
