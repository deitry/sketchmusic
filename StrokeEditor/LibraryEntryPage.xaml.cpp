//
// LibraryEntryPage.xaml.cpp
// Реализация класса LibraryEntryPage
//

#include <ppltasks.h>

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

void StrokeEditor::LibraryEntryPage::InitializePage()
{
	
}

void StrokeEditor::LibraryEntryPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	//e->
	//auto a = e->Content;
	auto args = reinterpret_cast<StrokeEditor::LibraryEntryNavigationArgs^>(e->Parameter);
	//this->_library = args->library;
	this->_entry = reinterpret_cast<SketchMusic::Idea^>(args->idea);
	if (this->_entry == nullptr)
	{
		this->_entry = ref new SketchMusic::Idea;
		this->DeleteBtn->IsEnabled = false;	// что не рождено, удалено быть не может
	}
	// пока других нету, задаём принудительно
	this->_entry->Category = SketchMusic::IdeaCategoryEnum::melody;
	//_isRead = args->isRead;

	// TODO : очень плохо, надо каким-то образом сделать через привязку, триггеры, хз
	entryNameTB->IsReadOnly = _isRead;
	categoryTB->IsReadOnly = true;	// выбора нет
	creationTB->IsReadOnly = true;
	//ratingTB->IsReadOnly = _isRead;
	descrTB->IsReadOnly = _isRead;
	serContTB->IsReadOnly = false; // TODO : заглушка, чтобы можно было вручную изменять текст, а в частности инструменты
	tagsTB->IsReadOnly = _isRead;
	//_isRead = args->isRead;
	//this->UpdateLayout();
	
	entryNameTB->Text = _entry->Name;
	categoryTB->Text = (String^)(ref new SketchMusic::VerboseIdeaCategoryToStrConverter())->Convert(_entry->Category, TypeName(Platform::String::typeid), nullptr, nullptr);
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
	this->_entry->Rating = ratingSlider->Value;
	long long time;
	_time64(&time);
	this->_entry->ModifiedTime = time;
	this->_entry->Tags = this->tagsTB->Text;
}


void StrokeEditor::LibraryEntryPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	SaveEntry();
	this->Frame->Navigate(TypeName(StrokeEditor::MelodyEditorPage::typeid), this->_entry);
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
	deleteFileDialog->Content = "Если вы подтвердите, то идея удалится безвозвратно. Вы уверены?";
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
	if (!((App^)App::Current)->UpdateIdea(this->_entry))
	{
		((App^)App::Current)->InsertIdea(this->_entry);
	}
}