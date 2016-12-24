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
	//for (int idx = 0; idx < count; idx++)
	//{
	//	columns[idx]
	//	data[idx]
	//}

//	Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
//		Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified);	// чтобы не десериализовывать контент
	//auto idea = ref new SketchMusic::Idea(
	//	std::stoi(data[0]),	// хэш
	//	StringFromAscIIChars(data[1]),	// имя
	//	static_cast<SketchMusic::IdeaCategoryEnum>(std::stoi(data[2])),	// категория
	//	StringFromAscIIChars(data[9]),	// описание				
	//	std::stoi(data[4]),	// родитель
	//	static_cast<byte>(std::stoi(data[10])),	// рейтинг
	//	StringFromAscIIChars(data[3]),	// контент
	//	StringFromAscIIChars(data[5]),	// теги
	//	StringFromAscIIChars(data[6]),	// проекты
	//	std::stol(data[7]),	// создан
	//	std::stol(data[8])	// изменён				
	//);
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

	// Работа с БД
	// int rc = sqlite3_exec(db, <SQL-запрос>, <КАЛЛБАК>, 0, &zErrMsg);
		
	sqlite3* db = ((App^)App::Current)->libraryDB;
	if (db)
	{
		char *zErrMsg = 0;
		
		// инициализация
		// - создание таблицы если нету
		int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS `summary` ("
									"`hash` INTEGER NOT NULL PRIMARY KEY,"
									"`name` TEXT,"
									"'category' INTEGER,"
									"'content' BLOB,"
									"'parent' INTEGER,"
									"'tags' TEXT,"
									"'projects' TEXT,"
									"'created' TEXT,"
									"'modified' TEXT,"
									"'description' TEXT,"
									"'rating' INTEGER);", NULL, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}

		// delete all
		//rc = sqlite3_exec(db, "DELETE FROM summary", NULL, 0, &zErrMsg);
		//if (rc != SQLITE_OK) {
		//	fprintf(stderr, "SQL error: %s\n", zErrMsg);
		//	sqlite3_free(zErrMsg);
		//}
		//rc = sqlite3_exec(db, "VACUUM", NULL, 0, &zErrMsg);
		//if (rc != SQLITE_OK) {
		//	fprintf(stderr, "SQL error: %s\n", zErrMsg);
		//	sqlite3_free(zErrMsg);
		//}
		
		// тестовые записи
		//auto idea = ref new SketchMusic::Idea("idea1", SketchMusic::IdeaCategoryEnum(1));
		//String^ sqlQuery = "INSERT INTO summary (hash,name,category,content,parent,tags,projects,created,modified,description,rating) "
		//	"VALUE (" + idea->Hash + idea->Name + idea->Category.ToString() + idea->Content->serializeToString() + idea->Parent + idea->Tags + idea->Projects +
		//	idea->CreationTime + idea->ModifiedTime + idea->Description + idea->Rating + ")";
		//idea->Content = ref new SketchMusic::Text("piano");
		//idea->Content->addSymbol(ref new SketchMusic::Cursor(1, 1),ref new SketchMusic::SNote(1));
		//idea->Content->addSymbol(ref new SketchMusic::Cursor(3, 1), ref new SketchMusic::SNote(1));
		//idea->Content->addSymbol(ref new SketchMusic::Cursor(3, 30), ref new SketchMusic::SNote(3));

		//itoa(static_cast<int>(idea->Category),str,10);
		//String^ sqlQuery = "INSERT INTO summary (hash,name,category,content,created,modified) "
		//	"VALUES (" + idea->Hash +",'"+ idea->Name + "'," + static_cast<int>(idea->Category).ToString() + "," +
		//				"'"+idea->Content->serialize()->Stringify() +"'," + idea->CreationTime + "," + idea->ModifiedTime + ")";
		//int size_needed = sqlQuery->Length();
		//char* charQuery = new char[size_needed + 1];
		//WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), charQuery, size_needed, NULL, NULL);
		////const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
		//charQuery[size_needed] = '\0';
		//rc = sqlite3_exec(db, charQuery, NULL, 0, &zErrMsg);
		//if (rc != SQLITE_OK) {
		//	fprintf(stderr, "SQL error: %s\n", zErrMsg);
		//	sqlite3_free(zErrMsg);
		//}
		//delete[] charQuery;

		// чтение объектов из БД
		//String^ sqlQuery = "SELECT * FROM summary";
		//int size_needed = sqlQuery->Length();
		//charQuery = new char[size_needed + 1];
		//WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), charQuery, size_needed, NULL, NULL);
		//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
		//charQuery[size_needed] = '\0';
		
		char* charQuery = "SELECT * FROM summary";
		rc = sqlite3_exec(db, charQuery, StrokeEditor::LibraryOverviewPage::sqlite_readentry_callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		//delete[] charQuery;
	}

	LibView->ItemsSource = ideaLibrary;

	// тестовая библиотека
	/*ideaLibrary = ref new Vector<SketchMusic::Idea^>();
	ideaLibrary->Append(ref new SketchMusic::Idea("idea1", SketchMusic::IdeaCategoryEnum(1)));
	ideaLibrary->Append(ref new SketchMusic::Idea("idea2", SketchMusic::IdeaCategoryEnum(2)));
	ideaLibrary->Append(ref new SketchMusic::Idea("idea3", SketchMusic::IdeaCategoryEnum(10)));

	LibView->ItemsSource = ideaLibrary;*/
}

void StrokeEditor::LibraryOverviewPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	// если в качестве параметра нам передали идею, то или заменяем в списке уже имеющуюся,
	// или создаём новую
	SketchMusic::Idea^ idea = (SketchMusic::Idea^) e->Parameter;
	if (!idea) return;	// если ничего не передали - ничего не делаем

	if (idea->CreationTime == 0 && idea->ModifiedTime == 0)
	{
		// нам поручили удалить эту запись
		// TODO : указания к действию на основе передачи типа через аргументы
		DeleteIdea(idea);
		RefreshBtn_Click(this, ref new RoutedEventArgs);
		return;
	}

	// иначе пробиваем в базе по хешу
	for (auto i : ideaLibrary)
	{
		if (i->Hash == idea->Hash)
		{
			i = idea;	// сработает?
			UpdateIdea(idea);
			return;
		}
	}

	// если не заменили, то добавляем новую идею в библиотеку
	InsertIdea(idea);
	ideaLibrary->Append(idea);	// вью перерисуется?
}

int StrokeEditor::LibraryOverviewPage::InsertIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = ((App^)App::Current)->libraryDB;
	char *zErrMsg = 0;
	// отладить, если захочешь передавать всё
	//String^ sqlQuery = "INSERT INTO summary (hash,name,category,content,parent,tags,projects,created,modified,description,rating) "
	//	"VALUE (" + idea->Hash+"," + idea->Name + "," + idea->Category.ToString() + "," + idea->Content->serializeToString() + "," + idea->Parent + "," + idea->Tags + "," + idea->Projects + "," +
	//	idea->CreationTime + idea->ModifiedTime + idea->Description + idea->Rating + ")";
	String^ sqlQuery = "INSERT INTO summary (hash,name,category,content,description,created,modified) "
		"VALUES (" + idea->Hash +",'"+ idea->Name + "'," + static_cast<int>(idea->Category).ToString() + "," +
					"'"+idea->Content->serialize()->Stringify() +"','" +idea->Description + "'," + idea->CreationTime + "," + idea->ModifiedTime + ")";

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), NULL, 0, NULL, NULL);
	char* charQuery = new char[size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), charQuery, size_needed, NULL, NULL);
	//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
	charQuery[size_needed] = '\0';
	int rc = sqlite3_exec(db, charQuery, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	delete[] charQuery;
	return rc;
}

int StrokeEditor::LibraryOverviewPage::DeleteIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = ((App^)App::Current)->libraryDB;
	char *zErrMsg = 0;
	// отладить, если захочешь передавать всё
	//String^ sqlQuery = "INSERT INTO summary (hash,name,category,content,parent,tags,projects,created,modified,description,rating) "
	//	"VALUE (" + idea->Hash+"," + idea->Name + "," + idea->Category.ToString() + "," + idea->Content->serializeToString() + "," + idea->Parent + "," + idea->Tags + "," + idea->Projects + "," +
	//	idea->CreationTime + idea->ModifiedTime + idea->Description + idea->Rating + ")";
	String^ sqlQuery = "DELETE FROM summary WHERE "
		"hash=" + idea->Hash + ";";

	int size_needed = sqlQuery->Length();
	char* charQuery = new char[size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), charQuery, size_needed, NULL, NULL);
	//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
	charQuery[size_needed] = '\0';
	int rc = sqlite3_exec(db, charQuery, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	delete[] charQuery;
	return rc;
}

int StrokeEditor::LibraryOverviewPage::UpdateIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = ((App^)App::Current)->libraryDB;
	char *zErrMsg = 0;
	String^ sqlQuery = "UPDATE summary SET "
		"name='" + idea->Name + "',"
		"category=" + static_cast<int>(idea->Category).ToString() + ","
		"content='" + idea->SerializedContent + "',"
		"description='" + idea->Description + "',"
		"created=" + idea->CreationTime + ","
		"modified=" + idea->ModifiedTime +" " 
		"WHERE hash=" + idea->Hash + " ;";

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), NULL, 0, NULL, NULL);
	char* charQuery = new char[size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, sqlQuery->Data(), sqlQuery->Length(), charQuery, size_needed, NULL, NULL);
	//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
	charQuery[size_needed] = '\0';
	int rc = sqlite3_exec(db, charQuery, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	delete[] charQuery;
	return rc;
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
}


void StrokeEditor::LibraryOverviewPage::RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	sqlite3* db = ((App^)App::Current)->libraryDB;
	if (db)
	{
		ideaLibrary->Clear();
		char* charQuery = "SELECT * FROM summary";
		char* zErrMsg;
		int rc = sqlite3_exec(db, charQuery, StrokeEditor::LibraryOverviewPage::sqlite_readentry_callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		//delete[] charQuery;
		LibView->ItemsSource = ideaLibrary;
	}

	SizeTxt->Text = "Кол-во в массиве: " + ideaLibrary->Size + " + Кол-во в отображении: " + LibView->Items->Size;
	
	LibView->UpdateLayout();
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
	// открыть страницу со сведениями о данной идее
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(nullptr, false));
}
