//
// CompositionEditorPage.xaml.cpp
// Реализация класса CompositionEditorPage
//

#include "pch.h"
#include "CompositionEditorPage.xaml.h"

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
using namespace SketchMusic;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

CompositionEditorPage::CompositionEditorPage()
{
	InitializeComponent();

	//Object^ content = ref new SketchMusic::PositionedSymbol(ref new SketchMusic::Cursor, ref new SketchMusic::SNote(5));

	auto Notes = ref new Platform::Collections::Vector<PositionedSymbol^>;
	Notes->Append(ref new SketchMusic::PositionedSymbol(ref new Cursor, ref new SNote(6)));
	Notes->Append(ref new SketchMusic::PositionedSymbol(ref new Cursor, ref new SNote(10)));
	Notes->Append(ref new SketchMusic::PositionedSymbol(ref new Cursor, ref new SNote(13)));

	CurrentChordView->SetNotes(Notes);
}


void StrokeEditor::CompositionEditorPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//We should verify if there are pages in the navigation back stack 
	//before navigating to the previous page.
	if (this->Frame != nullptr && Frame->CanGoBack)
	{
		//Using the GoBack method, the frame navigates to the previous page.
		Frame->GoBack();
	}
}
