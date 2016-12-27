//
// LibraryOverviewPage.xaml.cpp
// Реализация класса LibraryOverviewPage
//

#include <string>
#include <locale>
#include <codecvt>

#include "pch.h"
#include "LibraryOverviewPage.xaml.h"
#include "LibraryEntryPage.xaml.h"
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

using Windows::Foundation::Collections::IVector;
using Platform::Collections::Vector;

Windows::Foundation::Collections::IVector<SketchMusic::Idea^>^ StrokeEditor::LibraryOverviewPage::ideaLibrary = nullptr;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238
inline String^ StringFromAscIIChars(char* chars) //StrokeEditor::LibraryOverviewPage::
{
	if (!chars) return nullptr;
	//
	//size_t newsize = strlen(chars) + 1;
	//wchar_t * wcstring = new wchar_t[newsize];
	//size_t convertedChars = 0;
	//mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
	//String^ str = ref new Platform::String(wcstring);
	//delete[] wcstring;
	//return str;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring intermediateForm = converter.from_bytes(chars);
	return ref new Platform::String(intermediateForm.c_str());
}

int StrokeEditor::LibraryOverviewPage::sqlite_readentry_callback(void *unused, int count, char **data, char **columns)
{
	auto idea = ref new SketchMusic::Idea;
	idea->Hash = data[0] ? std::stoi(data[0]) : 0;	// хэш	// не делаем проверок, пусть будет ошибка, если хэша нет
	idea->Name = StringFromAscIIChars(data[1]);	// имя
	idea->Category = static_cast<SketchMusic::IdeaCategoryEnum>(std::stoi(data[2]));	// категория
	idea->Description = StringFromAscIIChars(data[9]);	// описание				
	idea->Parent = data[4] ? std::stoi(data[4]) : 0;	// родитель
	idea->Rating = data[10] ? static_cast<byte>(std::stoi(data[10])) : 0;	// рейтинг
	idea->SerializedContent = StringFromAscIIChars(data[3]);	// контент
	idea->Tags = StringFromAscIIChars(data[5]);	// теги
	idea->Projects = StringFromAscIIChars(data[6]);	// проекты
	idea->CreationTime = data[7] ? std::stol(data[7]) : 0;	// создан
	idea->ModifiedTime= data[8] ? std::stol(data[8]) : 0;	// изменён	

	ideaLibrary->Append(idea);
	
	return 0;
}

LibraryOverviewPage::LibraryOverviewPage()
{
	InitializeComponent();

	ideaLibrary = ref new Vector<SketchMusic::Idea^>();
	LibView->ItemsSource = ideaLibrary;
}

void StrokeEditor::LibraryOverviewPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	// получение настроек навроде параметров фильтрации
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	if (localSettings->Values->HasKey("filter_last3days")) 
	{ this->last3DaysFilter = (bool)localSettings->Values->Lookup("filter_last3days"); }
	if (localSettings->Values->HasKey("filter_rating5Higher")) 
	{ this->rating5Higher = (bool)localSettings->Values->Lookup("filter_rating5Higher"); }
	if (localSettings->Values->HasKey("filter_todoTag")) 
	{ this->todoTagsFilter = (bool)localSettings->Values->Lookup("filter_todoTag"); }
	if (localSettings->Values->HasKey("filter_baseTag")) 
	{ this->baseTagsFilter = (bool)localSettings->Values->Lookup("filter_baseTag"); }
	if (localSettings->Values->HasKey("filter_notBaseTag")) 
	{ this->notBaseTagsFilter = (bool)localSettings->Values->Lookup("filter_notBaseTag"); }
	if (localSettings->Values->HasKey("sort_index")) 
	{ this->SortingList->SelectedIndex = (int)localSettings->Values->Lookup("sort_index"); }

	for (auto&& i : FilterList->Items)
	{
		FrameworkElement^ item = dynamic_cast<TextBlock^>(static_cast<Object^>(i));
		switch (std::stoi(((String^)item->Tag)->Data()))
		{
		case 0: if (last3DaysFilter) FilterList->SelectedItems->Append(i); break;
		case 1: if (rating5Higher) FilterList->SelectedItems->Append(i);break;
		case 2: if (todoTagsFilter) FilterList->SelectedItems->Append(i);break;
		case 3: if (baseTagsFilter) FilterList->SelectedItems->Append(i);break;
		case 4: if (notBaseTagsFilter) FilterList->SelectedItems->Append(i);break;
		}
	}

	// если в качестве параметра нам передали идею, то или заменяем в списке уже имеющуюся,
	// или создаём новую
	SketchMusic::Idea^ idea = (SketchMusic::Idea^) e->Parameter;
	if (idea)
	{
		// если помечено на удаление
		if (idea->CreationTime == 0 && idea->ModifiedTime == 0)
		{
			// нам поручили удалить эту запись
			// TODO : указания к действию на основе передачи типа через аргументы
			((App^)App::Current)->DeleteIdea(idea);
			RefreshList();
			return;
		}
		else
		{
			// изменения в идее
			int rc = ((App^)App::Current)->UpdateIdea(idea);
			if (rc != SQLITE_OK)
			{
				((App^)App::Current)->InsertIdea(idea);
			}
		}
	}

	RefreshList();
}

void StrokeEditor::LibraryOverviewPage::OnNavigatedFrom(NavigationEventArgs ^ e)
{
	// сохранение настроек навроде параметров фильтрации
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	localSettings->Values->Insert("filter_last3days",this->last3DaysFilter);
	localSettings->Values->Insert("filter_rating5Higher", this->rating5Higher);
	localSettings->Values->Insert("filter_todoTag", this->todoTagsFilter);
	localSettings->Values->Insert("filter_baseTag", this->baseTagsFilter);
	localSettings->Values->Insert("filter_notBaseTag", this->notBaseTagsFilter);

	localSettings->Values->Insert("sort_index", this->SortingList->SelectedIndex);
}



void StrokeEditor::LibraryOverviewPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//We should verify if there are pages in the navigation back stack 
	//before navigating to the previous page.
	if (this->Frame != nullptr && Frame->CanGoBack)
	{
		//Using the GoBack method, the frame navigates to the previous page.
		Frame->GoBack();
	}
}

void StrokeEditor::LibraryOverviewPage::RefreshList()
{
	// - Получить текущее значение выбранного фильтра
	// - Отправить в базу данных запрос на получение элементов согласно выбранному фильтру
	// - Создать объекты (Приспособленец?) для отдельных элементов (если их слишком много, то только для части?)

	// к способу отображения - биндить коллекцию, визуальные объекты пусть отображаются сами через заданный темплейт
	// Если будет не устраивать - тогда подумаем. Пока коллекция будет не чересчур большой, этого должно хватить.

	// возможность смены выбора способа отображения? Несколько темплейтов

	sqlite3* db = ((App^)App::Current)->libraryDB;
	if (db)
	{
		ideaLibrary->Clear();
		std::string charQuery;

		// сначала определяем фильтры, постепенно формируя строку запроса
		bool tmp = false;
		if (last3DaysFilter)
		{
			long long t;
			_time64(&t);
			charQuery = charQuery + "(modified > " + std::to_string(t - 60*60*24*3) + ")";
			tmp = true;
		}
		
		if (rating5Higher)
		{
			if (tmp) { charQuery = charQuery + " AND "; }
			else { tmp = true; }
			charQuery = charQuery + "(rating > " + std::to_string(5)+")";
		}
		
		if (todoTagsFilter)
		{ 
			if (tmp) { charQuery = charQuery + " AND "; }
			else { tmp = true; }
			charQuery = charQuery + "((tags LIKE '%todo%'))"; 
		}

		if (baseTagsFilter && !notBaseTagsFilter)
		{
			if (tmp) { charQuery = charQuery + " AND "; }
			else { tmp = true; }
			charQuery = charQuery + "((tags LIKE '%base%'))";
		}
		if (notBaseTagsFilter && !baseTagsFilter)
		{ 
			if (tmp) { charQuery = charQuery + " AND "; }
			else { tmp = true; }
			charQuery = charQuery + "((NOT (tags LIKE '%base%')) OR (tags IS NULL))";
		}

		if (charQuery != "") { charQuery = "SELECT * FROM summary WHERE " + charQuery; }
		else { charQuery = "SELECT * FROM summary"; }
		
		switch (this->SortingList->SelectedIndex)
		{
		case 1:
			charQuery += " ORDER BY rating DESC, modified DESC;";
			break;
		case 0:
		default: charQuery += " ORDER BY modified DESC, name ASC;"; break;
		}
		
		char* zErrMsg;
		int rc = sqlite3_exec(db, charQuery.c_str(), StrokeEditor::LibraryOverviewPage::sqlite_readentry_callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		LibView->ItemsSource = ideaLibrary;
	}

	//SizeTxt->Text = "Кол-во в массиве: " + ideaLibrary->Size + " + Кол-во в отображении: " + LibView->Items->Size;

	LibView->UpdateLayout();
}




void StrokeEditor::LibraryOverviewPage::RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RefreshList();
}


void StrokeEditor::LibraryOverviewPage::LibView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(e->ClickedItem,false));
}


void StrokeEditor::LibraryOverviewPage::GoMenuBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о данной идее
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid), e);
}


void StrokeEditor::LibraryOverviewPage::CreateEntryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть страницу со сведениями о новой идее
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(nullptr, false));
}


void StrokeEditor::LibraryOverviewPage::FilterList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	for (auto&& i : e->AddedItems)
	{
		//unsigned int index;
		//FilterList->Items->IndexOf(i, &index);
		FrameworkElement^ item = dynamic_cast<TextBlock^>(static_cast<Object^>(i));
		switch (std::stoi(((String^)item->Tag)->Data()))
		{
		case 0:
			// фильтрация по дате - последние три дня
			last3DaysFilter = true;
			break;
		case 1:
			rating5Higher = true;
			break;
		case 2:
			todoTagsFilter = true;
			break;
		case 3:
			baseTagsFilter = true;
			break;
		case 4:
			notBaseTagsFilter = true;
			break;
		}
	}

	for (auto&& i : e->RemovedItems)
	{
		//unsigned int index;
		FrameworkElement^ item = dynamic_cast<TextBlock^>(static_cast<Object^>(i));
		switch (std::stoi(((String^)item->Tag)->Data()))
		{
		case 0:
			// фильтрация по дате - последние три дня
			last3DaysFilter = false;
			break;
		case 1:
			rating5Higher = false;
			break;
		case 2:
			todoTagsFilter = false;
			break;
		case 3:
			baseTagsFilter = false;
			break;
		case 4:
			notBaseTagsFilter = false;
			break;
		}
	}
}
