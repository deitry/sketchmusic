//
// MelodyEditorPage.xaml.cpp
// Реализация класса MelodyEditorPage
//

#include "pch.h"
#include "MelodyEditorPage.xaml.h"
#include "concrt.h"
#include "LibraryEntryPage.xaml.h"
#include "CompositionEditorPage.xaml.h"
#include "ManageInstrumentsDialog.xaml.h"

using namespace StrokeEditor;
using namespace SketchMusic;
using namespace SketchMusic::View;
namespace SMC = SketchMusic::Commands;

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
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Data::Json;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

MelodyEditorPage::MelodyEditorPage()
{
	((App^)App::Current)->WriteToDebugFile("Инициализация окна редактирования текста");
	InitializeComponent();
	((App^)App::Current)->WriteToDebugFile("Инициализация компонентов завершена");
}

void StrokeEditor::MelodyEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	((App^)App::Current)->WriteToDebugFile("Переход на страницу");
	InitializePage();
	((App^)App::Current)->WriteToDebugFile("Инициализация окна завершена");

	auto args = dynamic_cast<MelodyEditorArgs^>(e->Parameter);
	_Sender = args->Sender;

	_idea = dynamic_cast<Idea^>(args->Args);
	if (_idea)
	{
		((App^)App::Current)->_CurrentIdea = _idea;
		LoadIdea();
	}
	else
	{
		_compositionArgs = dynamic_cast<CompositionNavigationArgs^>(args->Args);
		if (_compositionArgs)
		{
			((App^)App::Current)->_CurrentCompositionArgs = _compositionArgs;
			if (_compositionArgs->SelectedIdea)
			{
				_idea = _compositionArgs->SelectedIdea;
				LoadIdea();
			}
			else
			{
				LoadComposition();
			}
		}
	}

	((App^)App::Current)->WriteToDebugFile("Загрузка данных завершена");

	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	if (localSettings)
	{
		if (localSettings->Values->HasKey("need_metronome"))
		{
			((App^)App::Current)->_player->needMetronome = (bool)localSettings->Values->Lookup("need_metronome");
			_keyboard->SetKey(KeyType::metronome, ((App^)App::Current)->_player->needMetronome);
		}
		else {
			((App^)App::Current)->_player->needMetronome = true;
		}

		if (localSettings->Values->HasKey("need_play_generic"))
		{
			((App^)App::Current)->_player->needPlayGeneric = (bool)localSettings->Values->Lookup("need_play_generic");
			_keyboard->SetKey(KeyType::playGeneric, ((App^)App::Current)->_player->needPlayGeneric);
		}
		else {
			((App^)App::Current)->_player->needPlayGeneric = true;
		}

		if (localSettings->Values->HasKey("precount"))
		{
			Object^ obj = localSettings->Values->Lookup("precount");
			bool has; if (obj) has = true; else has = false;	// не знаю, почему с ним так сложно.
				// но почему-то тут периодически всё падает
			((App^)App::Current)->_player->precount = has ? 4 : 0;
			_keyboard->SetKey(KeyType::precount, ((App^)App::Current)->_player->precount);
		}
		else {
			((App^)App::Current)->_player->precount = 4;
		}
	}

	((App^)App::Current)->_player->StopAtLast = false;

	((App^)App::Current)->WriteToDebugFile("Загрузка настроек завершена");

	((App^)App::Current)->WriteToDebugFile("Почти всё");

	TextsList->DataContext = _texts->texts;
	TextsList->SelectedIndex = 0;

	PartsList->DataContext = _texts->getParts();
	
	viewType = ViewType::TextRow;

	((App^)App::Current)->WriteToDebugFile("Инициализация страницы завершена");

	_keyboard->Focus(Windows::UI::Xaml::FocusState::Programmatic);
}

void StrokeEditor::MelodyEditorPage::LoadIdea()
{
	((App^)App::Current)->WriteToDebugFile("Загрузка данных идеи");

	if (_idea->Content == nullptr)
	{
		// восстанавливаем из сериализованного состояния
		if (_idea->SerializedContent != nullptr)
		{
			_idea->Content = SketchMusic::CompositionData::deserialize(_idea->SerializedContent);
		}
		else
		{
			_idea->Content = ref new CompositionData();
			
			Windows::Storage::ApplicationDataContainer^ localSettings =
				Windows::Storage::ApplicationData::Current->LocalSettings;
			if (localSettings->Values->HasKey("default_instr"))
			{
				auto str = (String^)localSettings->Values->Lookup("default_instr");
				JsonObject^ json = ref new JsonObject;
				if (JsonObject::TryParse(str, &json))
				{
					auto instr = Instrument::Deserialize(json);
					if (instr)
						_idea->Content->texts->Append(ref new Text(instr));
				}
			}

			// если ключа не было или не смогли восстановить инструмент
			if (_idea->Content->texts->Size == 0)
				_idea->Content->texts->Append(ref new Text(ref new Instrument("grand_piano.sf2")));
		}
	}

	_texts = _idea->Content;
	_textRow->SetText(_texts, nullptr);
}

void StrokeEditor::MelodyEditorPage::LoadComposition()
{
	((App^)App::Current)->WriteToDebugFile("Загрузка данных композиции");

	// полагаем, что у композиции данные уже существуют
	if (_compositionArgs->Project->Data->texts->Size == 0)
	{
		Windows::Storage::ApplicationDataContainer^ localSettings =
			Windows::Storage::ApplicationData::Current->LocalSettings;
		
		if (localSettings->Values->HasKey("default_instr"))
		{
			auto str = (String^)localSettings->Values->Lookup("default_instr");
			JsonObject^ json = ref new JsonObject;
			if (JsonObject::TryParse(str, &json))
			{
				auto instr = Instrument::Deserialize(json);
				if (instr)
					_compositionArgs->Project->Data->texts->Append(ref new Text(instr));
			}
		}
		if (_compositionArgs->Project->Data->texts->Size == 0)
			_compositionArgs->Project->Data->texts->Append(ref new Text(ref new Instrument("grand_piano.sf2")));
	}

	_texts = _compositionArgs->Project->Data;
	_textRow->SetText(_texts, nullptr, _compositionArgs->Selected);

	partsItem->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

void StrokeEditor::MelodyEditorPage::InitializePage()
{
	((App^)App::Current)->WriteToDebugFile("Инициализация страницы");

	// подгоняем ширину TextRow
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width - MenuSplitView->CompactPaneLength;
	_textRow->Width = width;

	// Обработчики для команд
	AddSymHandler = ref new SMC::Handler([=](Object^ args) -> bool {

		SketchMusic::Commands::SymbolHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		AddSymbolToText(symArgs->_text, symArgs->_sym);
		
		return true;
	});

	DeleteSymHandler = ref new SMC::Handler([=](Object^ args) -> bool {

		SketchMusic::Commands::SymbolHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;
		
		DeleteSymbolFromText(symArgs->_text, symArgs->_sym);
		return true;
	});

	MoveSymHandler = ref new SMC::Handler([=](Object^ args) -> bool {

		SketchMusic::Commands::SymbolPairHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolPairHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// - изменяем положение символа
		symArgs->_text->moveSymbol(symArgs->_oldSym, symArgs->_newSym->_pos);
		// - меняем положение графического объекта
		_textRow->SetSymbolView(symArgs->_oldSym, symArgs->_newSym);
		return true;
	});
	
	ReverseMoveSymHandler = ref new SMC::Handler([=](Object^ args) -> bool {
		SketchMusic::Commands::SymbolPairHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolPairHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// - изменяем положение символа
		symArgs->_text->moveSymbol(symArgs->_newSym, symArgs->_oldSym->_pos);
		// - меняем положение графического объекта
		_textRow->SetSymbolView(symArgs->_newSym, symArgs->_oldSym);
		return true;
	});

	AddMultipleSymHandler = ref new SketchMusic::Commands::Handler([=](Object^ args) -> bool {
		SMC::MultiSymbolHandlerArgs^ symArgs = dynamic_cast<SMC::MultiSymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// проходимся по списку и добавляем каждый символ
		for (auto&& sym : symArgs->_OldSymbols)
		{
			AddSymbolToText(symArgs->_text, sym);
		}
		return true;
	});

	DeleteMultipleSymHandler = ref new SMC::Handler([=](Object^ args) -> bool {
		SMC::MultiSymbolHandlerArgs^ symArgs = dynamic_cast<SMC::MultiSymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// проходимся по списку и удаляем каждый символ
		for (auto&& sym : symArgs->_OldSymbols)
		{
			DeleteSymbolFromText(symArgs->_text, sym);
		}
		return true;
	});
	
	// Команды для манипуляции с текстом
	MoveSymCommand = ref new SMC::Command(MoveSymHandler, ReverseMoveSymHandler);
	AddSymCommand = ref new SMC::Command(AddSymHandler, DeleteSymHandler);
	DeleteSymCommand = ref new SMC::Command(DeleteSymHandler, AddSymHandler);
	DeleteMultipleSymCommand = ref new SMC::Command(DeleteMultipleSymHandler, AddMultipleSymHandler);

	AddTextCommand = ref new SMC::Command(AddTextHandler, DeleteTextHandler);
	DeleteTextCommand = ref new SMC::Command(DeleteTextHandler, AddTextHandler);
	
	_textRow->_CommandManager = ((App^)App::Current)->_manager;
	_textRow->AddSymCommand = AddSymCommand;
	_textRow->DeleteSymCommand = DeleteSymCommand;
	_textRow->MoveSymCommand = MoveSymCommand;

	(safe_cast<::SketchMusic::View::BaseKeyboard^>(this->_keyboard))->KeyPressed += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::MelodyEditorPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&MelodyEditorPage::_keyboard_KeyboardPressed);
	(safe_cast<::SketchMusic::View::BaseKeyboard^>(this->_keyboard))->KeyReleased += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::MelodyEditorPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&MelodyEditorPage::_keyboard_KeyReleased);
	
	curPosChangeToken = 
		((App^)App::Current)->_player->CursorPosChanged += 
		ref new Windows::Foundation::EventHandler<SketchMusic::Cursor ^>(this, &StrokeEditor::MelodyEditorPage::OnCursorPosChanged);
	playerStateChangeToken =
		((App^)App::Current)->_player->StateChanged += 
		ref new Windows::Foundation::EventHandler<SketchMusic::Player::PlayerState>(this, &StrokeEditor::MelodyEditorPage::OnStateChanged);

	KeyDownToken =
		Window::Current->CoreWindow->KeyDown 
		+= ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &StrokeEditor::MelodyEditorPage::OnKeyDown);

	KeyUpToken =
		Window::Current->CoreWindow->KeyUp 
		+= ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &StrokeEditor::MelodyEditorPage::OnKeyUp);
	//bpmChangeToken = 
	//	((App^)App::Current)->_player->BpmChanged += ref new Windows::Foundation::EventHandler<float>(this, &StrokeEditor::MelodyEditorPage::OnBpmChanged);

	// привязка состояния
	canUndoChangeToken =
		((App^)App::Current)->_manager->CanUndoChanged += 
		ref new Windows::Foundation::EventHandler<bool>(this, &StrokeEditor::MelodyEditorPage::OnCanUndoChanged);
	canRedoChangeToken =
		((App^)App::Current)->_manager->CanRedoChanged += 
		ref new Windows::Foundation::EventHandler<bool>(this, &StrokeEditor::MelodyEditorPage::OnCanRedoChanged);
	
	// подписка на событие нажатия ноты, чтобы её озвучить
	_textRow->SymbolPressed += ref new Windows::Foundation::EventHandler<SketchMusic::PositionedSymbol ^>(this, &StrokeEditor::MelodyEditorPage::OnSymbolPressed);
}


void StrokeEditor::MelodyEditorPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// остановить проигрывание, если идёт
	((App^)App::Current)->_player->cycling = false;
	((App^)App::Current)->_player->stop();
	((App^)App::Current)->_player->stopKeyboard();
	
	// сохраняем настройки
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	// отписка от события
	(((App^)App::Current)->_player)->StateChanged -= playerStateChangeToken;
	(((App^)App::Current)->_player)->CursorPosChanged -= curPosChangeToken;

	// очищаем менеджер команд
	((App^)App::Current)->_manager->Clear();

	if (_idea)
	{
		// сохраняем данные в виде текста
		_idea->SerializedContent = _idea->Content->serialize()->Stringify();
	}

	// проверка на композицию раньше, потому что если редактируем идею внутри композиции, у нас может быть и то, и другое
	if (_compositionArgs)
	{
		this->Frame->Navigate(TypeName(StrokeEditor::CompositionEditorPage::typeid), _compositionArgs);
	}
	else if (_idea)
	{	
		this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(_idea, true));
	}
}


void StrokeEditor::MelodyEditorPage::SaveData()
{
	if (_compositionArgs)
	{
		// сохранить в файл
		// сериализуем композицию
		auto json = _compositionArgs->Project->Serialize();
		// пишем в файл
		create_task(FileIO::WriteTextAsync(_compositionArgs->File, json->Stringify()))
			.then([=] {
			auto dialog = ref new ContentDialog;
			dialog->Title = "Сохранение в файл";
			dialog->Content = "Успешно сохранено";
			dialog->PrimaryButtonText = "Ок";
			return dialog->ShowAsync();
		});
	}
	else if (_idea)
	{
		// сохранить данные - сериализовать
		_idea->SerializedContent = _idea->Content->serialize()->Stringify();
		
		// обновить modified time
		long long time;
		_time64(&time);
		_idea->ModifiedTime = time;

		// отправить в библиотеку
		if (!((App^)App::Current)->UpdateIdea(_idea))
		{
			((App^)App::Current)->InsertIdea(_idea);
		}

		auto dialog = ref new ContentDialog;
		dialog->Title = "Сохранение в библиотеку";
		dialog->Content = "Успешно сохранено";
		dialog->PrimaryButtonText = "Ок";
		create_task(dialog->ShowAsync());
	}
}

void StrokeEditor::MelodyEditorPage::playAll_Click()
{
	if (((App^)App::Current)->_player->_state != SketchMusic::Player::PlayerState::STOP)
	{
		((App^)App::Current)->_player->stop();
		((App^)App::Current)->WriteToDebugFile("Остановка плеера");
		return;
	}

	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		auto async = concurrency::create_task([this]
		{
			//((App^)App::Current)->_player->stop();
			((App^)App::Current)->WriteToDebugFile("Запуск плеера");
			((App^)App::Current)->_player->playText(this->_texts, ref new SketchMusic::Cursor(_textRow->currentPosition));
		});
	}));
}

void StrokeEditor::MelodyEditorPage::_keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ args)
{
	if (_textRow->current == nullptr) return;

	if (args->key)
	{
		switch (args->key->type)
		{
		case SketchMusic::View::KeyType::metronome:
		{
			((App^)App::Current)->_player->needMetronome = (bool)args->key->value;
			Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert("need_metronome", ((App^)App::Current)->_player->needMetronome);
			break;
		}
		case SketchMusic::View::KeyType::playGeneric:
		{
			((App^)App::Current)->_player->needPlayGeneric = (bool)args->key->value;
			Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert("need_play_generic", ((App^)App::Current)->_player->needMetronome);
			break;
		}
		case SketchMusic::View::KeyType::quantization:
		{
			((App^)App::Current)->_player->quantize = args->key->value;
			_textRow->quantize = args->key->value;
			break;
		}
		case SketchMusic::View::KeyType::precount:
		{
			((App^)App::Current)->_player->precount = args->key->value;
			Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert("precount", ((App^)App::Current)->_player->needMetronome);
			break;
		}
		case SketchMusic::View::KeyType::end:
		case SketchMusic::View::KeyType::note:
		case SketchMusic::View::KeyType::relativeNote:
		case SketchMusic::View::KeyType::genericNote:
		{
			SketchMusic::ISymbol^ sym = SketchMusic::ISymbolFactory::CreateSymbol(args->key->type, args->key->value);

			auto snote = dynamic_cast<SketchMusic::SNote^>(sym);
			if (snote)
			{
				SketchMusic::Player::NoteOff^ noteOff = ref new SketchMusic::Player::NoteOff;
				((App^)App::Current)->_player->playSingleNote(snote, _textRow->current->instrument, 0, noteOff);
				// сохраняем noteOff где-нибудь, ассоциированно с нажатой клавишей
				notesPlayingMap.insert(std::make_pair(args->key, noteOff));
			}

			// надо добавлять сразу аккордами
			if (recording)
			{
				
				Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
				{
					auto delay = concurrency::create_task([=]
					{
						// небольшая задержка, чтобы можно было нажимать чуть-чуть заранее
						concurrency::wait(20);

						Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=] {
							// создаём команду на добавление ноты в текст и сохраняем её в истории
							((App^)App::Current)->_manager->AddAndExecute(
								AddSymCommand,
								ref new SMC::SymbolHandlerArgs(
									_textRow->current,
									ref new PositionedSymbol(
										ref new SketchMusic::Cursor(((App^)App::Current)->_player->quantize ? _textRow->currentPosition : ((App^)App::Current)->_player->_cursor), 
										sym)));
						}));
					});
				}));
				//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				//	+ " / tick = " + _textRow->currentPosition->getTick();

				if (!appending && ((App^)App::Current)->_player->_state != SketchMusic::Player::PlayerState::PLAY)
				{
					appending = true;
					Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
					{
						auto delay = concurrency::create_task([this]
						{
							unsigned int timeout = 600 * 60 / ((App^)App::Current)->_player->_BPM / _textRow->scale;
							concurrency::wait(timeout);
							Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=] {
								this->appending = false;
								if (this->viewType == ViewType::TextRow)
								{
									this->_textRow->MoveCursorRight();
								}
								else
								{
									this->MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
								}
							}));

						});
					}));
				}
			}
			break;
		}
		case SketchMusic::View::KeyType::tempo:
		{
			SketchMusic::ISymbol^ sym = ref new SketchMusic::STempo(args->key->value);

			// создаём команду на добавление ноты в текст и сохраняем её в истории
			((App^)App::Current)->_manager->AddAndExecute(
				AddSymCommand,
				ref new SMC::SymbolHandlerArgs(
					_texts->ControlText, 
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym)));
			break;
		}
		case SketchMusic::View::KeyType::enter:
			// перенос строки обрабатываем на отпускании
			break;
		case SketchMusic::View::KeyType::move:
		{
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=] 
			{
				if (args->key->value >= 0)
				{
					if (viewType == ViewType::TextRow)
						_textRow->MoveCursorRight();
					else MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
				}
				else
				{
					if (viewType == ViewType::TextRow) _textRow->MoveCursorLeft();
					else MoveLeftCWBtn_Click(this, ref new RoutedEventArgs);
				}
			}));
			break;
		}
		case SketchMusic::View::KeyType::record:
			recording = args->key->value;
			((App^)App::Current)->_player->recording = recording;
			break;
		case SketchMusic::View::KeyType::backspace:
		{
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=] 
			{
				Backspace();
			}));
			break;
		}
		case SketchMusic::View::KeyType::stop:
			((App^)App::Current)->_player->stop();
			((App^)App::Current)->_player->stopKeyboard();
			break;
		case SketchMusic::View::KeyType::play:
			playAll_Click();
			break;
		case SketchMusic::View::KeyType::cycling:
			((App^)App::Current)->_player->cycling = args->key->value;
			((App^)App::Current)->_player->StopAtLast = ((App^)App::Current)->_player->cycling;
			break;
		case SketchMusic::View::KeyType::zoom:
			_textRow->SetScale(args->key->value * 2);
			break;
		case SketchMusic::View::KeyType::hide:
			//_textRow->Height = 
			break;
		case SketchMusic::View::KeyType::eraser:
			_textRow->EraserTool = args->key->value;
			break;
		case SketchMusic::View::KeyType::octave:
		case SketchMusic::View::KeyType::space:
		default:
			break;
		}
	}
}

void StrokeEditor::MelodyEditorPage::_keyboard_KeyReleased(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ args)
{
	// на отпускание клавиши мы должны
	// - прекращать звучание
	for (auto noteIter = notesPlayingMap.begin(); noteIter != notesPlayingMap.end(); noteIter++)
	{
		if (((*noteIter).first->type == args->key->type) && ((*noteIter).first->value == args->key->value))
		{
			(*noteIter).second->Cancel();
			notesPlayingMap.erase(noteIter);
			break;
		}
	}

	// обрабатывание событий отпускания некоторых клавиш
	switch (args->key->type)
	{
	case KeyType::enter:
	{
		int beat = _textRow->currentPosition->Beat;
		if (_textRow->currentPosition->Tick > 0) beat++;
		// создаём команду на добавление ноты в текст и сохраняем её в истории
		auto newpos = ref new SketchMusic::Cursor(beat);
		((App^)App::Current)->_manager->AddAndExecute(
			AddSymCommand,
			ref new SMC::SymbolHandlerArgs(_texts->ControlText,
				ref new PositionedSymbol(newpos, ref new SNewLine)));
		_textRow->SetCursor(newpos);
		break;
	}
	}
}

void StrokeEditor::MelodyEditorPage::OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args)
{
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
		((App^)App::Current)->WriteToDebugFile("Обработка изменения состояния плеера");
		switch (args)
		{
		case SketchMusic::Player::PlayerState::PLAY:
			this->_keyboard->OnKeyboardStateChanged(this, ref new SketchMusic::View::KeyboardState(SketchMusic::View::KeyboardStateEnum::play));
			PlayItemTxt1->Text = L"\uE769";
			PlayItemTxt2->Text = L"Остановить";
			break;
		case SketchMusic::Player::PlayerState::STOP:
		case SketchMusic::Player::PlayerState::WAIT:
			this->_keyboard->OnKeyboardStateChanged(this, ref new SketchMusic::View::KeyboardState(SketchMusic::View::KeyboardStateEnum::stop));
			PlayItemTxt1->Text = L"\uE768";
			PlayItemTxt2->Text = L"Воспроизвести";
			break;
		}
	}));
}


void StrokeEditor::MelodyEditorPage::OnCursorPosChanged(Platform::Object ^sender, SketchMusic::Cursor^ pos)
{
	create_task(this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Low,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
		_textRow->SetCursor(pos);
	})));
}

void StrokeEditor::MelodyEditorPage::ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	Text^ text = dynamic_cast<Text^>(TextsList->SelectedItem); // e->AddedItems->First()->Current
	if (text)
	{
		if (text != _textRow->current)
		{
			_textRow->SetText(_texts, text, _compositionArgs ? _compositionArgs->Selected : -1);
		}
		else
		{
			// даём возможность поменять инструмент текста
			auto dialog = ref new ManageInstrumentsDialog;

			create_task(dialog->ShowAsync())
				.then([=](ContentDialogResult result)
			{
				if (result == ContentDialogResult::Primary)
				{
					this->Dispatcher->RunAsync(
						Windows::UI::Core::CoreDispatcherPriority::Normal,
						ref new Windows::UI::Core::DispatchedHandler([=]() {

						Windows::Storage::ApplicationDataContainer^ localSettings =
							Windows::Storage::ApplicationData::Current->LocalSettings;

						this->_texts->texts->Append(ref new Text(dialog->Selected));

					}));
				}
			});
		}
	}
	TextsFlyout->Hide();
}

void StrokeEditor::MelodyEditorPage::ListView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	auto instr = dynamic_cast<Instrument^>(e->ClickedItem);
	if (instr)
	{
		// спрашиваем у плеера, загружен ли такой инструмент и есть ли у него пресеты
		// - выводим окошко со списком доступных пресетов
		this->Dispatcher->RunAsync(
			Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([=]() {
			
			auto init = concurrency::create_task([=]
			{
				// такая обёртка, если асинхронно будем читать данные, если они ешё не созданы
				return ((App^)App::Current)->_player->GetSFData(instr);
			}).then([=](SketchMusic::SFReader::SFData^ data)->void {
				this->Dispatcher->RunAsync(
					Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([=]() {
				
					if (data == nullptr) return;

					//auto data = sfdata.get();
					ContentDialog^ presetListDlg = ref new ContentDialog;
					Grid^ grid = ref new Grid;
					grid->ColumnDefinitions->Append(ref new ColumnDefinition);
					grid->ColumnDefinitions->Append(ref new ColumnDefinition);
					ListView^ list = ref new ListView;
					list->ItemTemplate = (DataTemplate^) this->Resources->Lookup("myResourceTemplate");
					list->ItemsSource = data->presets;
					list->IsItemClickEnabled = true;
					list->ItemClick += ref new Windows::UI::Xaml::Controls::ItemClickEventHandler([=](Object^, Object^)
					{
						presetListDlg->Hide();
					});
					Button^ btn = ref new Button;
					btn->Content = "Отмена";
					btn->Click += ref new Windows::UI::Xaml::RoutedEventHandler([=](Object^, Object^)
					{
						presetListDlg->Hide();
					});

					grid->Children->Append(list);
					grid->Children->Append(btn);
					btn->SetValue(Grid::ColumnProperty, 1);

					presetListDlg->Content = grid;

					auto dialog = concurrency::create_task(presetListDlg->ShowAsync());
					dialog.then([=](concurrency::task<ContentDialogResult> t)
					{
						ContentDialogResult result = t.get();

						if (list->SelectedItem)// result == ContentDialogResult::Primary)
						{
							auto preset = dynamic_cast<SketchMusic::SFReader::SFPreset^>(list->SelectedItem);
							if (preset)
							{
								this->_texts->texts->Append(ref new Text(ref new Instrument(instr->_name, instr->FileName, preset->name)));
								list->SelectedItem = nullptr;
							}
						}
					});
				}));
			});
		}));
	}
	//InstrumentsFlyout->Hide();
}


void StrokeEditor::MelodyEditorPage::DeleteTextBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto list = dynamic_cast<ListView^>(TextsFlyout->Content);
	if (list)
	{
		Text^ text = dynamic_cast<Text^>(list->SelectedItem);
		auto size = _texts->texts->Size;
		if (text && (size > 1)) // последний удалить нельзя
		{
			unsigned int index;
			_texts->texts->IndexOf(text, &index);
			unsigned int new_index = index;
			if (new_index> (size - 2)) { new_index = size - 2; }	// size-1 = крайний элемент; size-1-1 = крайний элемент после удаления
			TextsList->SelectedIndex = new_index;
			_texts->texts->RemoveAt(index);
			_textRow->SetText(_texts, _texts->texts->GetAt(new_index), (_compositionArgs ?_compositionArgs->Selected : -1));
		}
		DeleteTextFlyout->Hide();
	}
}


void StrokeEditor::MelodyEditorPage::Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width
		- MenuSplitView->CompactPaneLength - _textRow->Margin.Left - _textRow->Margin.Right;
	_textRow->Width = width;
}


// переместить курсор к ближайшей левой ноте
void StrokeEditor::MelodyEditorPage::MoveLeftCWBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Cursor^ prevPos = nullptr;
	auto notes = PrevChord->GetNotes();
	if (notes == nullptr || notes->Size == 0) notes = PrevGChord->GetNotes();
	if (notes == nullptr || notes->Size == 0) return;

	//if (notes->Size > 0) // учли выше
	prevPos = notes->GetAt(0)->_pos;
	
	if (prevPos)
	{
		UpdateChordViews(prevPos);
	}
}


// переместить курсор к ближайшей правой ноте
void StrokeEditor::MelodyEditorPage::MoveRightCWBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Cursor^ nextPos = nullptr;
	auto notes = NextChord->GetNotes();
	if (notes == nullptr || notes->Size == 0) notes = NextGChord->GetNotes();
	if (notes == nullptr || notes->Size == 0) return;

	//if (notes->Size > 0)
	nextPos = notes->GetAt(0)->_pos;
	
	if (nextPos)
	{
		UpdateChordViews(nextPos);
	}
}

void StrokeEditor::MelodyEditorPage::UpdateChordViews(Cursor ^ pos)
{
	if (this->viewType != ViewType::ChordView) return;

	auto txt = _textRow->current;
	if (txt == nullptr) return;

	//auto cur = _textRow->currentPosition; 
	_textRow->SetCursor(pos);

	CurPosTxt->Text = L"" + pos->Beat + ":" + pos->Tick;
	// сюда - только обычные ноты
	CurrentChord->SetNotes(txt->getNotesAt(pos, SymbolType::NOTE));
	// сюда - все, кроме обычных
	auto notes = txt->getNotesAtExcluding(pos, SymbolType::NOTE);
	// так сложно, потому что часть нот попадает в ControlText
	auto notes2 = _texts->ControlText->getNotesAtExcluding(pos, SymbolType::NOTE);
	for (auto&& note : notes2)
	{
		notes->Append(note);
	}
	CurrentGChord->SetNotes(notes);

	auto prevPos = txt->getPosAtLeft(pos);
	auto prevPos2 = _texts->ControlText->getPosAtRight(pos);	// TODO : можно сделать лучше
	if (prevPos2 && prevPos2->GT(prevPos)) prevPos = prevPos2;

	PrevChord->SetNotes(txt->getNotesAt(prevPos, SymbolType::NOTE));
	notes = txt->getNotesAtExcluding(prevPos, SymbolType::NOTE);
	// так сложно, потому что часть нот попадает в ControlText
	notes2 = _texts->ControlText->getNotesAtExcluding(prevPos, SymbolType::NOTE);
	for (auto&& note : notes2)
	{
		notes->Append(note);
	}
	PrevGChord->SetNotes(notes);

	bool prevEnabled = (PrevChord->GetNotes() == nullptr) ? false : (PrevChord->GetNotes()->Size > 0);
	if (prevEnabled == false) prevEnabled = (PrevGChord->GetNotes() == nullptr) ? false : (PrevGChord->GetNotes()->Size > 0);
	
	if (prevEnabled)
	{
		MoveLeftCWBtn->IsEnabled = true; PrevPosTxt->Text = L"" + prevPos->Beat + ":" + prevPos->Tick ;
	}
	else { MoveLeftCWBtn->IsEnabled = false; PrevPosTxt->Text = ""; }

	auto nextPos = txt->getPosAtRight(pos);
	auto nextPos2 = _texts->ControlText->getPosAtRight(pos);	// TODO : можно сделать лучше
	if (nextPos2 && nextPos2->LT(nextPos)) nextPos = nextPos2;

	NextChord->SetNotes(_textRow->current->getNotesAt(nextPos, SymbolType::NOTE));
	notes = txt->getNotesAtExcluding(nextPos, SymbolType::NOTE);
	// так сложно, потому что часть нот попадает в ControlText
	notes2 = _texts->ControlText->getNotesAtExcluding(nextPos, SymbolType::NOTE);
	for (auto&& note : notes2)
	{
		notes->Append(note);
	}
	NextGChord->SetNotes(notes);
	
	bool nextEnabled = (NextChord->GetNotes() == nullptr) ? false : (NextChord->GetNotes()->Size > 0);
	if (nextEnabled == false) nextEnabled = (NextGChord->GetNotes() == nullptr) ? false : (NextGChord->GetNotes()->Size > 0);

	if (nextEnabled)
	{
		MoveRightCWBtn->IsEnabled = true; NextPosTxt->Text = L"" + nextPos->Beat + ":" + nextPos->Tick;
	}
	else { MoveRightCWBtn->IsEnabled = false; NextPosTxt->Text = ""; }

}

void StrokeEditor::MelodyEditorPage::PartsList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (PartsList->SelectedItem)
	{
		_textRow->SetText(_texts, _textRow->current, PartsList->SelectedIndex);
		_compositionArgs->Selected = PartsList->SelectedIndex;
	}
}


void StrokeEditor::MelodyEditorPage::MenuButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if ((HamburgerButton == (ContentControl^)e->OriginalSource) )//|| (HamburgerButton2 == (ContentControl^)e->OriginalSource))
	{
		bool isOpen = !MenuSplitView->IsPaneOpen;
		MenuSplitView->IsPaneOpen = isOpen;
		MenuSeparator->X2 = (isOpen ? menu->ActualWidth : 22.);
		MenuSeparator2->X2 = (isOpen ? menu->ActualWidth : 22.);
		MenuSeparator3->X2 = (isOpen ? menu->ActualWidth : 22.);
	}
	else if (homeItem == (ContentControl^)e->OriginalSource)
	{
		GoBackBtn_Click(this, nullptr);
	}
	else if (SaveItem == (ContentControl^)e->OriginalSource)
	{
		SaveData();
	}
	else if (textsItem == (ContentControl^)e->OriginalSource)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(textsCtrl);
	}
	else if (partsItem == (ContentControl^)e->OriginalSource)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(partsCtrl);
	}
	else if (addItem == (ContentControl^)e->OriginalSource)
	{
		//Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(addCtrl);
		auto dialog = ref new ManageInstrumentsDialog;

		create_task(dialog->ShowAsync())
			.then([=](ContentDialogResult result)
		{
			if (result == ContentDialogResult::Primary)
			{
				this->Dispatcher->RunAsync(
					Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([=]() {

					Windows::Storage::ApplicationDataContainer^ localSettings =
						Windows::Storage::ApplicationData::Current->LocalSettings;

					this->_texts->texts->Append(ref new Text(dialog->Selected));

				}));
			}
		});
	}
	else if (deleteItem == (ContentControl^)e->OriginalSource)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(deleteCtrl);
	}
	else if (PlayItem == (ContentControl^)e->OriginalSource)
	{
		playAll_Click();
	}
	else if (ChangeViewItem == (ContentControl^)e->OriginalSource)
	{
		// TODO : если типов представления нот будет больше, сделать через флайаут или контентдиалог
		viewType = viewType == ViewType::TextRow ? ViewType::ChordView : ViewType::TextRow;

		// Соответствующим образом меняем представление
		switch (viewType)
		{
		case ViewType::TextRow:
			_textRow->Visibility = Windows::UI::Xaml::Visibility::Visible;
			_ChordViewGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			break;
		case ViewType::ChordView:
			_textRow->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			_ChordViewGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
			UpdateChordViews(_textRow->currentPosition);
			break;
		}

	}
	else if (settingsItem == (ContentControl^)e->OriginalSource)
	{
		//myFrame->Navigate(settingsItem->GetType());
		bool need = !SettingsView->IsPaneOpen;
		if (need && MenuSplitView->IsPaneOpen)
		{
			MenuSplitView->IsPaneOpen = false;
		}
		SettingsView->IsPaneOpen = need;
	}
}

void StrokeEditor::MelodyEditorPage::UndoItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	((App^)App::Current)->_manager->Undo();
}


void StrokeEditor::MelodyEditorPage::RedoItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	((App^)App::Current)->_manager->Redo();
}

void StrokeEditor::MelodyEditorPage::AddSymbolToText(SketchMusic::Text ^ text, SketchMusic::PositionedSymbol ^ symbol)
{
	// добавляем символ в текст
	auto type = symbol->_sym->GetSymType();
	if (type == SymbolType::TEMPO || type == SymbolType::NPART || type == SymbolType::CLEF)
	{
		// темп и может какие-нибудь другие символы будем заменять вместо добавления новых в ту же точку
		text->addOrReplaceSymbol(symbol);
	}
	else text->addSymbol(symbol);

	// добавляем визуальное представление символа
	// - TextRow
	_textRow->AddSymbolView(text, symbol);
	// - ChordView
	if (symbol->_sym->GetSymType() == SymbolType::NOTE)
	{
		auto arr = this->CurrentChord->GetNotes();
		if (arr)
			arr->Append(symbol);
	}
	else
	{
		auto arr = this->CurrentGChord->GetNotes();
		if (arr)
			arr->Append(symbol);
	}
}

void StrokeEditor::MelodyEditorPage::DeleteSymbolFromText(SketchMusic::Text ^ text, SketchMusic::PositionedSymbol ^ symbol)
{
	// удаляем символ из текста
	text->deleteSymbol(symbol->_pos, symbol->_sym);

	// удаляем графическое представление
	_textRow->DeleteSymbolView(symbol);
	// TODO : удаление символа из ChordView
}

// Перемещение курсора назад и удаление всех символов, 
// попавших в диапазон между исходным и конечным положением курсора
void StrokeEditor::MelodyEditorPage::Backspace()
{
	// если в текущем положении есть разрыв строки, сперва удаляем его
	auto curSyms = _texts->ControlText->GetSymbols(_textRow->currentPosition, _textRow->currentPosition);
	for (auto sym : curSyms)
	{
		if (sym->_sym->GetSymType() == SymbolType::NLINE)
		{
			((App^)App::Current)->_manager->AddAndExecute(
				DeleteSymCommand,
				ref new SMC::SymbolHandlerArgs(_texts->ControlText,
					sym));
			return;
		}
	}

	// иначе - переводим курсор назад и смотрим, какие символы "попались"
	auto currentPos = ref new Cursor(_textRow->currentPosition);
	_textRow->MoveCursorLeft();
	auto newPos = ref new Cursor(_textRow->currentPosition);

	// выбираем все символы из текста между этими двумя положениями
	auto selectedSyms = _textRow->current->GetSymbols(newPos, currentPos);
	if (selectedSyms->Size > 0)
	{
		((App^)App::Current)->_manager->AddAndExecute(
			DeleteMultipleSymCommand,
			ref new SMC::MultiSymbolHandlerArgs(_textRow->current,
				selectedSyms));
	}
}


void StrokeEditor::MelodyEditorPage::OnCanRedoChanged(Platform::Object ^sender, bool args)
{
	RedoItem->IsEnabled = args;
}


void StrokeEditor::MelodyEditorPage::OnCanUndoChanged(Platform::Object ^sender, bool args)
{
	UndoItem->IsEnabled = args;
}


void StrokeEditor::MelodyEditorPage::OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args)
{
	if (args->KeyStatus.WasKeyDown == true)
		return;

	using Windows::System::VirtualKey;
	bool isCtrl = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Control) != Windows::UI::Core::CoreVirtualKeyStates::None);
	
	if (ctrlPressed)
	{
		switch (args->VirtualKey)
		{
		case VirtualKey::Z:
			((App^)App::Current)->_manager->Undo();
			break;
		case VirtualKey::Y:
			((App^)App::Current)->_manager->Redo();
			break;
		case VirtualKey::S:
			SaveData();
			break;
		}
	}
	else if (args->VirtualKey == VirtualKey::Control)
	{
		ctrlPressed = true;
	}
}


void StrokeEditor::MelodyEditorPage::OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args)
{
	using Windows::System::VirtualKey;
	if (args->VirtualKey == VirtualKey::Control)
		ctrlPressed = false;
}

// Озвучиваем нажатый на textRow символ
void StrokeEditor::MelodyEditorPage::OnSymbolPressed(Platform::Object ^sender, SketchMusic::PositionedSymbol ^args)
{
	auto snote = dynamic_cast<SketchMusic::SNote^>(args->_sym);
	if (snote)
	{
		((App^)App::Current)->_player->playSingleNote(snote, _textRow->current->instrument, 500, nullptr);
	}
}


void StrokeEditor::MelodyEditorPage::TextsList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	Text^ text = dynamic_cast<Text^>(e->ClickedItem); // TextsList->SelectedItem
	if (text)
	{
		if (text != _textRow->current)
		{
			_textRow->SetText(_texts, text, _compositionArgs ? _compositionArgs->Selected : -1);
		}
		else
		{
			// даём возможность поменять инструмент текста
			auto dialog = ref new ManageInstrumentsDialog;

			create_task(dialog->ShowAsync())
				.then([=](ContentDialogResult result)
			{
				if (result == ContentDialogResult::Primary)
				{
					this->Dispatcher->RunAsync(
						Windows::UI::Core::CoreDispatcherPriority::Normal,
						ref new Windows::UI::Core::DispatchedHandler([=]() {

						Windows::Storage::ApplicationDataContainer^ localSettings =
							Windows::Storage::ApplicationData::Current->LocalSettings;

						_textRow->current->instrument = dialog->Selected;
						//TextsList->ItemsSource = _texts;
					}));
				}
			});
		}
	}
	TextsFlyout->Hide();
}


void StrokeEditor::MelodyEditorPage::TextsFlyout_Opened(Platform::Object^ sender, Platform::Object^ e)
{
	int sel = TextsList->SelectedIndex;
	TextsList->DataContext = nullptr;
	TextsList->DataContext = _texts->texts;
	TextsList->SelectedIndex = sel;
}
