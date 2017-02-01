//
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
using namespace Windows::Storage;

using Windows::Foundation::Collections::IVector;
using Platform::Collections::Vector;


// Шаблон пустого приложения задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234227

/// <summary>
/// Инициализирует одноэлементный объект приложения.  Это первая выполняемая строка разрабатываемого
/// кода; поэтому она является логическим эквивалентом main() или WinMain().
/// </summary>
App::App()
{
	InitializeComponent();
	InitializeApp();
	((App^)App::Current)->WriteToDebugFile("Инициализация приложения завершена");
}

void StrokeEditor::App::InitializeApp()
{
	concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFileAsync("Debug.txt", CreationCollisionOption::ReplaceExisting))
		.then([=] (StorageFile^ file) {
		DebugFile = file;
	});

	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
	Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	// TODO : в качестве аргумента - или ещё лучше (?) в качестве возвращаемого значения - передавать хендлер
	// копируем файл из выбранной нами папки

	OpenLocalOrExternalLibrary("ideaLibrary.db");

	((App^)App::Current)->ideaLibrary = ref new Vector<SketchMusic::Idea^>();

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
		Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->SetPreferredMinSize(Size(300, 300));
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
		Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->SetPreferredMinSize(Size(300, 300));
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
	auto deferral = e->SuspendingOperation->GetDeferral();
	
	// Сохранить состояние приложения и остановить все фоновые операции
	SaveData();

	CloseLibrary();

	deferral->Complete();
}

void StrokeEditor::App::OnResuming(Platform::Object ^ sender, Platform::Object ^ args)
{
	OpenLocalOrExternalLibrary("ideaLibrary.db");
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

void StrokeEditor::App::WriteToDebugFile(Platform::String ^ str)
{
	if (DebugFile)
	{
		__time64_t long_time;
		_time64(&long_time);
		struct tm t;
		_localtime64_s(&t, &long_time);
		t.tm_year += 1900;
		t.tm_mon += 1;
		Platform::String^ timeStr = t.tm_year + "." +
			((t.tm_mon < 10) ? "0" : "") + t.tm_mon + "." +
			((t.tm_mday < 10) ? "0" : "") + t.tm_mday + " " +
			t.tm_hour + ":" + ((t.tm_min < 10) ? "0" : "") + t.tm_min + ":" +
			((t.tm_sec<10) ? "0" : "") + t.tm_sec;

		FileIO::AppendLinesAsync(DebugFile, ref new Platform::Collections::Vector<String^>(1,timeStr + " " + str));
	}
}

void StrokeEditor::App::OpenLibrary(Platform::String ^ dbPath)
{
	WriteToDebugFile("Открытие файла " + dbPath);
	// создание базы данных, если отсутствует
	//sqlite3* libraryDB; // объявление в хидере
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, dbPath->Data(), dbPath->Length(), NULL, 0, NULL, NULL);
	char *pathC = new char[size_needed + 1];
	WideCharToMultiByte(CP_UTF8, 0, dbPath->Data(), dbPath->Length(), pathC, size_needed, NULL, NULL);
	//const char* pathC = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fullPath->Data()).c_str();
	pathC[size_needed] = '\0';

	if (sqlite3_open(pathC, &libraryDB))
	{
		WriteToDebugFile("Ошибка открытия/создания БД");
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
			WriteToDebugFile("Ошибка создания таблицы");
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}

	delete[] pathC;
}

void StrokeEditor::App::SaveLibrary(Platform::String ^ dbName)
{
	// копируем библиотеку из кэшированной папки, если указано место её хранения
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	// если нет соответствующего ключа -> мы храним библиотеку в локальной папке и сохранять ничего не надо
	if (ideaLibrary && ideaLibrary->Size>0 && localSettings->Values->HasKey("lib_path"))
	{
		String^ targetPath = (String^)localSettings->Values->Lookup("lib_path");
		// копируем файл в целевую папку
		concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalCacheFolder->TryGetItemAsync(dbName))
			.then([=](IStorageItem^ item)
		{
			StorageFile^ file = dynamic_cast<StorageFile^>(item);
			if (file)
			{
				concurrency::create_task(StorageFolder::GetFolderFromPathAsync(targetPath))
					.then([=](StorageFolder^ targetFolder) {
					if (targetFolder)
					{
						file->CopyAsync(targetFolder,
							file->Name,
							Windows::Storage::NameCollisionOption::ReplaceExisting);
					}
				});
			}
		});
	}
}

void StrokeEditor::App::CloseLibrary()
{
	sqlite3_close(libraryDB);
}

void StrokeEditor::App::OpenLocalOrExternalLibrary(Platform::String ^ dbName)
{
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;
	if (localSettings->Values->HasKey("lib_path"))
	{
		String^ targetPath = (String^)localSettings->Values->Lookup("lib_path");
		// копируем файл в папку приложения
		try
		{
			concurrency::create_task(StorageFolder::GetFolderFromPathAsync(targetPath)
			).then([=](StorageFolder^ folder) {
				try
				{
					if (folder)
					{
						concurrency::create_task(folder->TryGetItemAsync(dbName))
							.then([=] (IStorageItem^ item) {

							StorageFile^ file = dynamic_cast<StorageFile^>(item);
							if (file)
							{
								concurrency::create_task(
									file->CopyAsync(Windows::Storage::ApplicationData::Current->LocalCacheFolder,
										file->Name,
										Windows::Storage::NameCollisionOption::ReplaceExisting))
									.then([=](StorageFile^ file) {
									OpenLibrary(Windows::Storage::ApplicationData::Current->LocalCacheFolder->Path + "\\" + dbName);
								});
							}
							else
							{
								WriteToDebugFile("Не найден файл библиотеки по адресу " + folder->Path);
								OpenLibrary(Windows::Storage::ApplicationData::Current->LocalFolder->Path + "\\" + dbName);
							}
						});
					}
				}
				catch (...)
				{

				}
			});
		}
		catch (...)
		{
			OpenLibrary(Windows::Storage::ApplicationData::Current->LocalFolder->Path + "\\" + dbName);
		}
	}
	else
	{
		OpenLibrary(Windows::Storage::ApplicationData::Current->LocalFolder->Path + "\\" + dbName);
	}
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

void StrokeEditor::App::SaveData()
{
	// TODO : сохранять копии? или во временные файлы?
	if (_CurrentIdea)
	{
		// сохранить данные - сериализовать
		if (_CurrentIdea->Content && _CurrentIdea->Content->HasContent())
			_CurrentIdea->SerializedContent = _CurrentIdea->Content->serialize()->Stringify();
		else return;

		// обновить modified time
		long long time;
		_time64(&time);
		_CurrentIdea->ModifiedTime = time;

		// отправить в библиотеку
		if (!((App^)App::Current)->UpdateIdea(_CurrentIdea))
		{
			((App^)App::Current)->InsertIdea(_CurrentIdea);
		}
	}
	else if (_CurrentCompositionArgs && _CurrentCompositionArgs->Project && _CurrentCompositionArgs->File)
	{
		// сохранить в файл
		// применяем изменения
		if (_CurrentParts)
			_CurrentCompositionArgs->Project->Data->ApplyParts(_CurrentParts);

		// сериализуем композицию
		auto json = _CurrentCompositionArgs->Project->Serialize();
		// пишем в файл
		FileIO::WriteTextAsync(_CurrentCompositionArgs->File, json->Stringify());
	}
	
	// теперь "сохранение"-копирование делается после каждой значимой операции с библиотекой, так что сохранение на выходе уже излишне
	//SaveLibrary("ideaLibrary.db");
}
// Установить на плитке строку с последним открытым элементом
// - передавать сюда название файла для композиции
// - или название идеи
void StrokeEditor::App::SetTile(Platform::String ^ str)
{
	//// Get an XmlDocument containing the badge template
	//auto badgeXml = BadgeUpdateManager::GetTemplateContent(BadgeTemplateType::BadgeNumber);
	//// Get the "badge" element
	//auto badgeElement = (XmlElement^)badgeXml->GetElementsByTagName("badge")->First();
	//// Set the count in its "value" attribute (as string)
	//badgeElement->SetAttribute("value", str);
	//// Create a badge notification from the XML content.
	//auto badgeNotification = ref new BadgeNotification(badgeXml);
	//// Send the badge notification to the app's tile.
	//BadgeUpdateManager::CreateBadgeUpdaterForApplication()->Update(badgeNotification);

	// Get an XML DOM version of a specific template by using getTemplateContent.
	auto tileXml = TileUpdateManager::GetTemplateContent(TileTemplateType::TileWide310x150Text03);

	// You will need to look at the template documentation to know how many text fields a particular template has.
	// Get the text attribute for this template and fill it in.
	auto  tileAttributes = tileXml->GetElementsByTagName("text");
	tileAttributes->GetAt(0)->AppendChild(tileXml->CreateTextNode(str));

	// Create the notification from the XML.
	auto tileNotification = ref new TileNotification(tileXml);

	// Send the notification to the calling app's tile.
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tileNotification);
}

void StrokeEditor::App::Play(SketchMusic::CompositionData^ texts, SketchMusic::Cursor ^ cursor)
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