﻿//
// LibraryEntryPage.xaml.cpp
// Реализация класса LibraryEntryPage
//

#include "pch.h"
#include "LibraryEntryPage.xaml.h"
#include "MelodyEditorPage.xaml.h"
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

LibraryEntryPage::LibraryEntryPage()
{
	InitializeComponent();
}

void StrokeEditor::LibraryEntryPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	//e->
	auto args = reinterpret_cast<StrokeEditor::LibraryEntryNavigationArgs^>(e->Parameter);
	this->_entry = reinterpret_cast<SketchMusic::Idea^>(args->idea);
	_isRead = args->isRead;

	// TODO : очень плохо, надо каким-то образом сделать через привязку, триггеры, хз
	entryNameTB->IsReadOnly = _isRead;
	categoryTB->IsReadOnly = _isRead;
	creationTB->IsReadOnly = _isRead;
	ratingTB->IsReadOnly = _isRead;
	descrTB->IsReadOnly = _isRead;
	serContTB->IsReadOnly = _isRead;	//this->Resources->Insert("_isRead", Platform::Boolean(args->isRead));
	//_isRead = args->isRead;
	//this->UpdateLayout();

	Binding^ entryNameBinding = ref new Binding();
	entryNameBinding->Source = _entry->Name;
	entryNameTB->SetBinding(entryNameTB->TextProperty, entryNameBinding);

	Binding^ categoryBinding = ref new Binding();
	categoryBinding->Source = _entry->Category;
	categoryBinding->Converter = ref new SketchMusic::IdeaCategoryToStrConverter();
	categoryTB->SetBinding(categoryTB->TextProperty, categoryBinding);

	Binding^ serContBinding = ref new Binding();
	serContBinding->Source = _entry->SerializedContent;
	serContTB->SetBinding(serContTB->TextProperty, serContBinding);
	
	//Binding^ entryNameBinding = ref new Binding();
	//entryNameBinding->Source = _entry->Name;
	//entryNameTB->SetBinding(entryNameTB->TextProperty, entryNameBinding);
	
	this->_entry->RaisePropertyChanged();
}


void StrokeEditor::LibraryEntryPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//We should verify if there are pages in the navigation back stack 
	//before navigating to the previous page.
	if (this->Frame != nullptr && Frame->CanGoBack)
	{
		//Using the GoBack method, the frame navigates to the previous page.
		Frame->GoBack();
	}
}


void StrokeEditor::LibraryEntryPage::NotifyBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->_entry->RaisePropertyChanged();
}


void StrokeEditor::LibraryEntryPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	this->Frame->Navigate(TypeName(StrokeEditor::MelodyEditorPage::typeid), this->_entry);
}


void StrokeEditor::LibraryEntryPage::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryOverviewPage::typeid), this->_entry);
}
