﻿//
// LibraryEntryPage.xaml.cpp
// Реализация класса LibraryEntryPage
//

#include <ppltasks.h>
#include <string>

#include "pch.h"
#include "LibraryEntryPage.xaml.h"
#include "MelodyEditorPage.xaml.h"
#include "LibraryOverviewPage.xaml.h"
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

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

LibraryEntryPage::LibraryEntryPage()
{
	InitializeComponent();

	InitializePage();
}

void StrokeEditor::LibraryEntryPage::InitializePage()
{
	auto vect = ref new Platform::Collections::Vector<IdeaCategoryEnum>;
	vect->Append((IdeaCategoryEnum)1);
	vect->Append((IdeaCategoryEnum)2);
	vect->Append((IdeaCategoryEnum)6);
	vect->Append((IdeaCategoryEnum)10);
	vect->Append((IdeaCategoryEnum)11);
	vect->Append((IdeaCategoryEnum)13);
	vect->Append((IdeaCategoryEnum)15);
	vect->Append((IdeaCategoryEnum)16);
	vect->Append((IdeaCategoryEnum)20);
	vect->Append((IdeaCategoryEnum)30);
	vect->Append((IdeaCategoryEnum)90);
	vect->Append((IdeaCategoryEnum)100);
	categoryCombo->ItemsSource = vect;
}

void StrokeEditor::LibraryEntryPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	auto args = reinterpret_cast<StrokeEditor::LibraryEntryNavigationArgs^>(e->Parameter);
	
	this->_entry = reinterpret_cast<SketchMusic::Idea^>(args->idea);
	if (this->_entry == nullptr)
	{
		this->_entry = ref new SketchMusic::Idea;
		this->DeleteBtn->IsEnabled = false;	// что не рождено, удалено быть не может
	}

	((App^)App::Current)->_CurrentIdea = this->_entry;

	// пока других нету, задаём принудительно
	this->_entry->Category = SketchMusic::IdeaCategoryEnum::melody;

	// TODO : очень плохо, надо каким-то образом сделать через привязку, триггеры, хз
	entryNameTB->IsReadOnly = _isRead;
	//categoryTB->IsReadOnly = true;	// выбора нет
	creationTB->IsReadOnly = true;
	descrTB->IsReadOnly = _isRead;
	serContTB->IsReadOnly = true;
	tagsTB->IsReadOnly = _isRead;
	//_isRead = args->isRead;
	//this->UpdateLayout();
	
	entryNameTB->Text = _entry->Name;
	auto converter = (VerboseIdeaCategoryToStrConverter^) this->Resources->Lookup("VerbCategory2Str");
	//categoryTB->Text = (String^)converter->Convert(_entry->Category, TypeName(Platform::String::typeid), nullptr, nullptr);
	
	for (auto&& item : categoryCombo->Items)
	{
		auto cat = static_cast<SketchMusic::IdeaCategoryEnum>(static_cast<Object^>(item));
		if (cat == _entry->Category)
		{
			categoryCombo->SelectedItem = item;
			break;
		}
	}

	descrTB->Text = _entry->Description;
	tagsTB->Text = _entry->Tags;
	serContTB->Text = _entry->SerializedContent;
	//Binding^ entryNameBinding = ref new Binding();
	//entryNameBinding->Source = _entry->Name;
	//entryNameTB->SetBinding(entryNameTB->TextProperty, entryNameBinding);

	//Binding^ categoryBinding = ref new Binding();
	//categoryBinding->Source = _entry->Category;
	//categoryBinding->Converter = ref new SketchMusic::VerboseIdeaCategoryToStrConverter();
	//categoryTB->SetBinding(categoryTB->TextProperty, categoryBinding);

	//Binding^ serContBinding = ref new Binding();
	//serContBinding->Source = _entry->SerializedContent;
	//serContTB->SetBinding(serContTB->TextProperty, serContBinding);
	
	//Binding^ descrBinding = ref new Binding();
	//descrBinding->Source = _entry->Description;
	//descrTB->SetBinding(descrTB->TextProperty, descrBinding);

	//Binding^ tagsBinding = ref new Binding();
	//tagsBinding->Source = _entry->Tags;
	//tagsTB->SetBinding(tagsTB->TextProperty, tagsBinding);

	ratingSlider->Value = _entry->Rating;
	
	//this->_entry->RaisePropertyChanged();
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
	//this->_entry->RaisePropertyChanged();
}

void StrokeEditor::LibraryEntryPage::SaveEntry()
{
	this->_entry->Name = entryNameTB->Text;
	this->_entry->Description = descrTB->Text;
	this->_entry->Rating = static_cast<unsigned char>(std::floor(ratingSlider->Value));
	long long time;
	_time64(&time);
	this->_entry->ModifiedTime = time;
	this->_entry->Tags = this->tagsTB->Text;
}


void StrokeEditor::LibraryEntryPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	SaveEntry();
	this->Frame->Navigate(TypeName(StrokeEditor::MelodyEditorPage::typeid), ref new MelodyEditorArgs(this, this->_entry));
}


void StrokeEditor::LibraryEntryPage::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// идём на вкладку с обзором данных, сохраняя изменения
	// TODO : использовать привязки

	SaveEntry();
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryOverviewPage::typeid), this->_entry);
}


void StrokeEditor::LibraryEntryPage::CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// идём на вкладку с обзором данных с пустыми руками
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryOverviewPage::typeid), nullptr);
}


void StrokeEditor::LibraryEntryPage::DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// TODO : сделать указания к действию через передачу параметров
	ContentDialog^ deleteFileDialog = ref new ContentDialog();
	
	deleteFileDialog->Title = "Удалить идею?";
	deleteFileDialog->Content = "Идея будет удалена безвозвратно. Вы уверены?";
	deleteFileDialog->PrimaryButtonText = "Удалить";
	deleteFileDialog->SecondaryButtonText = "Отмена";
	
	auto dialog = concurrency::create_task(deleteFileDialog->ShowAsync());
	dialog.then([=] (concurrency::task<ContentDialogResult> t)
	{
		ContentDialogResult result = t.get();

		// Delete the file if the user clicked the primary button. 
		/// Otherwise, do nothing. 
		if (result == ContentDialogResult::Primary)
		{
			this->_entry->CreationTime = 0;
			this->_entry->ModifiedTime = 0;

			this->Frame->Navigate(TypeName(StrokeEditor::LibraryOverviewPage::typeid), this->_entry);
		}
	});
	
	
}


void StrokeEditor::LibraryEntryPage::ExtendBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// создать новую идею, открыть новое окно
	auto extEntry = ref new SketchMusic::Idea();
	if (this->_entry != nullptr)
	{
		extEntry->Name = "Развитие " + this->_entry->Name;
		extEntry->Parent = this->_entry->Parent;
		extEntry->Content = this->_entry->Content;
		extEntry->SerializedContent = this->_entry->SerializedContent;
		extEntry->Category = this->_entry->Category;
		extEntry->Tags = this->_entry->Tags;
		extEntry->Description = this->_entry->Description;
	}
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(extEntry, false));
}

// сохранение идеи без перехода к обзору библиотеки
void StrokeEditor::LibraryEntryPage::SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	SaveEntry();

	int result = ((App^)App::Current)->InsertIdea(this->_entry); 
	if (result != SQLITE_OK)
	{
		result = ((App^)App::Current)->UpdateIdea(this->_entry);
	}

	if (result != SQLITE_OK)
	{
		auto dialog = ref new ContentDialog;
		dialog->Title = "Ошибка сохранения";
		dialog->Content = "Ошибка сохранения, код " + result;
		dialog->PrimaryButtonText = "Ок";
		concurrency::create_task(dialog->ShowAsync());
	}
	else
	{
		auto dialog = ref new ContentDialog;
		dialog->Title = "Сохранение в библиотеку";
		dialog->Content = "Успешно сохранено";
		dialog->PrimaryButtonText = "Ок";
		concurrency::create_task(dialog->ShowAsync());
	}
}

void StrokeEditor::LibraryEntryPage::HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid));
}


void StrokeEditor::LibraryEntryPage::_this_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}
