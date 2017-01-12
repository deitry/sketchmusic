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
}



void StrokeEditor::CompositionEditorPage::CompositionPartList_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	//if (!e->Data->Properties->HasKey("sourceList")) return;
	//auto sourceList = dynamic_cast<ListView^>(e->Data->Properties->Lookup("sourceList"));
	//if (sourceList == nullptr && sourceList != CompositionPartList) return;
	//
	//if (!e->Data->Properties->HasKey("sourceItem")) return;
	//auto sourceItem = dynamic_cast<SNewPart^>(e->Data->Properties->Lookup("sourceItem"));
	//if (sourceItem == nullptr) return;

	auto rearrangedParts = ref new Platform::Collections::Vector<PositionedSymbol^>;

	for (auto&& child : CompositionPartList->ItemsPanelRoot->Children)
	{
		FrameworkElement^ el = dynamic_cast<FrameworkElement^>(static_cast<Object^>(child));
		if (el == nullptr) continue;

		auto psym = dynamic_cast<PositionedSymbol^>(el->DataContext);
		if (psym)
		{
			rearrangedParts->Append(psym);
		}
	}

	CompositionView->SetParts(rearrangedParts);
	//CompositionView->UpdateView();
}


void StrokeEditor::CompositionEditorPage::CompositionPartList_DragItemsStarting(Platform::Object^ sender, Windows::UI::Xaml::Controls::DragItemsStartingEventArgs^ e)
{
	//if (!e->Items->Size) return;
	//
	//auto dataItem = e->Items->First();
	//e->Data->Properties->Insert("sourceItem", dataItem);
	//e->Data->Properties->Insert("sourceList", sender);
}


void StrokeEditor::CompositionEditorPage::CompositionPartList_DropCompleted(Windows::UI::Xaml::UIElement^ sender, Windows::UI::Xaml::DropCompletedEventArgs^ args)
{
	auto rearrangedParts = ref new Platform::Collections::Vector<PositionedSymbol^>;

	for (auto&& child : CompositionPartList->ItemsPanelRoot->Children)
	{
		FrameworkElement^ el = dynamic_cast<FrameworkElement^>(static_cast<Object^>(child));
		if (el == nullptr) continue;

		auto psym = dynamic_cast<PositionedSymbol^>(el->DataContext);
		if (psym)
		{
			rearrangedParts->Append(psym);
		}
	}

	CompositionView->SetParts(rearrangedParts);
	//CompositionView->UpdateView();
}


void StrokeEditor::CompositionEditorPage::Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{

}
