﻿//
// App.xaml.cpp
// Реализация класса приложения.
//

#include "pch.h"
#include "MainMenuPage.xaml.h"

#include <cvt/wstring>
#include <codecvt>

using namespace StrokeEditor;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace Windows::Data::Xml::Dom;

// Шаблон пустого приложения задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234227

/// <summary>
/// Инициализирует одноэлементный объект приложения.  Это первая выполняемая строка разрабатываемого
/// кода; поэтому она является логическим эквивалентом main() или WinMain().
/// </summary>
App::App()
{
	InitializeComponent();
	InitializeApp();
}

void StrokeEditor::App::InitializeApp()
{
	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
	Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	// TODO : в качестве аргумента - или ещё лучше (?) в качестве возвращаемого значения - передавать хендлер
	OpenLibrary("\\ideaLibrary.db");

	// создание глобальных объектов
	auto init = concurrency::create_task([this]
	{
		((App^)App::Current)->_player = ref new SketchMusic::Player::Player;
	});
	((App^)App::Current)->_manager = ref new SketchMusic::Commands::CommandManager;
}

/// <summary>
/// Вызывается при обычном запуске приложения пользователем.	Будут использоваться другие точки входа,
/// будет использоваться, например, если приложение запускается для открытия конкретного файла.
/// </summary>
/// <param name="e">Сведения о запросе и обработке запуска.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{

#if _DEBUG
		// Отображение сведений о профиле графики во время отладки.
		if (IsDebuggerPresent())
		{
			// Отображение текущих счетчиков частоты смены кадров
			 DebugSettings->EnableFrameRateCounter = true;
		}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	// Не повторяйте инициализацию приложения, если в окне уже имеется содержимое,
	// только обеспечьте активность окна
	if (rootFrame == nullptr)
	{
		// Создайте фрейм, используемый в качестве контекста навигации, и свяжите его с
		// ключом SuspensionManager
		rootFrame = ref new Frame();

		// Задайте язык по умолчанию
		rootFrame->Language = Windows::Globalization::ApplicationLanguages::Languages->GetAt(0);

		rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
		{
			// TODO: Восстановление сохраненного состояния сеанса только при необходимости и планирование
			// шаги по окончательному запуску только после завершения восстановления

		}

		if (rootFrame->Content == nullptr)
		{
			// Если стек навигации не восстанавливается для перехода к первой странице,
			// настройка новой страницы путем передачи необходимой информации в качестве параметра
			// навигации
			rootFrame->Navigate(TypeName(MainMenuPage::typeid), e->Arguments);
		}
		// Размещение фрейма в текущем окне
		Window::Current->Content = rootFrame;
		// Обеспечение активности текущего окна
		Window::Current->Activate();
	}
	else
	{
		if (rootFrame->Content == nullptr)
		{
			// Если стек навигации не восстанавливается для перехода к первой странице,
			// настройка новой страницы путем передачи необходимой информации в качестве параметра
			// навигации
			rootFrame->Navigate(TypeName(MainMenuPage::typeid), e->Arguments);
		}
		// Обеспечение активности текущего окна
		Window::Current->Activate();
	}
}

/// <summary>
/// Вызывается при приостановке выполнения приложения.	Состояние приложения сохраняется
/// без учета информации о том, будет ли оно завершено или возобновлено с неизменным
/// содержимым памяти.
/// </summary>
/// <param name="sender">Источник запроса приостановки.</param>
/// <param name="e">Сведения о запросе приостановки.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void) sender;	// Неиспользуемый параметр
	(void) e;	// Неиспользуемый параметр

	// Сохранить состояние приложения и остановить все фоновые операции
	sqlite3_close(libraryDB);
}

void StrokeEditor::App::OnResuming(Platform::Object ^ sender, Platform::Object ^ args)
{
	OpenLibrary("\\ideaLibrary.db");
}

/// <summary>
/// Вызывается в случае сбоя навигации на определенную страницу
/// </summary>
/// <param name="sender">Фрейм, для которого произошел сбой навигации</param>
/// <param name="e">Сведения о сбое навигации</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void StrokeEditor::App::OpenLibrary(Platform::String ^ dbName)
{
	// создание базы данных, если отсутствует
	//sqlite3* libraryDB; // объявление в хидере
	String^ fullPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path + dbName;
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, fullPath->Data(), fullPath->Length(), NULL, 0, NULL, NULL);
	char *pathC = new char[size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, fullPath->Data(), fullPath->Length(), pathC, size_needed, NULL, NULL);
	//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
	pathC[size_needed] = '\0';

	if (sqlite3_open(pathC, &libraryDB))
	{
		fprintf(stderr, "Ошибка открытия/создания БД: %s\n", sqlite3_errmsg(libraryDB));
		sqlite3_close(libraryDB);
	}

	if (libraryDB)
	{
		char *zErrMsg = 0;

		// инициализация
		// - создание таблицы если нету
		// Удостоверяемся, что БД содержит те колонки, что нам нужны
		int rc = sqlite3_exec(libraryDB, "CREATE TABLE IF NOT EXISTS `summary` ("
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
	}

	delete[] pathC;
}

int StrokeEditor::App::InsertIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = this->libraryDB;
	char *zErrMsg = 0;

	String^ sqlQuery = L"INSERT INTO summary (hash,name,category,content,parent,tags,projects,created,modified,description,rating) "
		"VALUES (" + idea->Hash + ",'" + idea->Name + "'," + static_cast<int>(idea->Category).ToString() + ",'" + idea->SerializedContent + "'," + idea->Parent +
		",'" + idea->Tags + "','" + idea->Projects + "'," + idea->CreationTime + "," + idea->ModifiedTime + ",'" + idea->Description + "'," + idea->Rating + ");";

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

int StrokeEditor::App::DeleteIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = this->libraryDB;
	char *zErrMsg = 0;
	
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

void StrokeEditor::App::ShowNotification(Platform::String ^ message)
{
	auto templ = Windows::UI::Notifications::ToastTemplateType::ToastText01;
	auto toastXml = Windows::UI::Notifications::ToastNotificationManager::GetTemplateContent(templ);
	auto textNodes = toastXml->GetElementsByTagName("text");
	for (auto i : textNodes)
	{
		String^ text = message;
		i->AppendChild(toastXml->CreateTextNode(text));
	}

	auto toast = ref new ToastNotification(toastXml);
	ToastNotificationManager::CreateToastNotifier()->Show(toast);
}

void StrokeEditor::App::Play(Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ texts, SketchMusic::Cursor ^ cursor)
{
	//auto async = concurrency::create_task([this,texts, cursor]
	//{
		this->_player->stop();
		this->_player->playText(texts, cursor);
	//});
}

int StrokeEditor::App::UpdateIdea(SketchMusic::Idea ^ idea)
{
	sqlite3* db = this->libraryDB;
	char *zErrMsg = 0;
	String^ sqlQuery = "UPDATE summary SET "
		"name='" + idea->Name + "',"
		"category=" + static_cast<int>(idea->Category).ToString() + ","
		"content='" + idea->SerializedContent + "',"
		"tags='" + idea->Tags + "',"
		"projects='" + idea->Projects + "',"
		"parent=" + idea->Parent + ","
		"rating=" + idea->Rating + ","
		"description='" + idea->Description + "',"
		"created=" + idea->CreationTime + ","
		"modified=" + idea->ModifiedTime + " "
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