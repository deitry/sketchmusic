//
// MelodyEditorPage.xaml.cpp
// Реализация класса MelodyEditorPage
//

#include "pch.h"
#include "MelodyEditorPage.xaml.h"
#include "concrt.h"
#include "LibraryEntryPage.xaml.h"
#include "CompositionEditorPage.xaml.h"

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

	_idea = dynamic_cast<Idea^>(e->Parameter);
	if (_idea) LoadIdea();
	else
	{
		_compositionArgs = dynamic_cast<CompositionNavigationArgs^>(e->Parameter);
		if (_compositionArgs) LoadComposition();
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
			((App^)App::Current)->_player->precount = ((bool)obj) ? 4 : 0;
			_keyboard->SetKey(KeyType::precount, ((App^)App::Current)->_player->precount);
		}
		else {
			((App^)App::Current)->_player->precount = 4;
		}
	}

	((App^)App::Current)->_player->StopAtLast = false;

	((App^)App::Current)->WriteToDebugFile("Загрузка настроек завершена");

	auto async = create_task([=]
	{
		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		return create_task(folder->GetFolderAsync("SketchMusic\\resources\\"));
	}).then([=](task<StorageFolder^> resourceFolderTask)->task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^ >
	{
		auto resourceFolder = resourceFolderTask.get();
		return create_task(resourceFolder->GetFilesAsync());
	}).then([=](task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^> resourceListTask)
	{
		availableInstruments = ref new Platform::Collections::Vector<Instrument^>;
		auto list = resourceListTask.get();
		for (auto&& file : list)
		{
			availableInstruments->Append(ref new Instrument(file->Name));
		}
	});

	//((App^)App::Current)->ShowNotification("Почти");
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
	{
		async.get();
		auto listView = dynamic_cast<ListView^>(InstrumentsFlyout->Content);
		if (listView)
		{
			listView->DataContext = availableInstruments;
		}

		//((App^)App::Current)->ShowNotification("Составлен список инструментов");
	}));

	ListView^ list = dynamic_cast<ListView^>(TextsFlyout->Content);
	if (list)
	{
		list->DataContext = _texts->texts;
		list->SelectedIndex = 0;
	}

	ListView^ list2 = dynamic_cast<ListView^>(PartsFlyout->Content);
	if (list2)
	{
		list2->DataContext = _texts->getParts();
	}
	
	viewType = ViewType::TextRow;
	//UpdateChordViews(_textRow->currentPosition);
	((App^)App::Current)->WriteToDebugFile("Инициализация страницы завершена");
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
			// принудительно, пока нет выбора других инструментов
			_idea->Content = ref new CompositionData();
			_idea->Content->texts->Append(ref new Text(ref new Instrument("grand_piano.sf2")));
			//_idea->Content->texts->Append(TestData::CreateTestText(2350));
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
		// принудительно, пока нет выбора других инструментов
		_compositionArgs->Project->Data->texts->Append(ref new Text(ref new Instrument("grand_piano.sf2")));
	}

	_texts = _compositionArgs->Project->Data;
	_textRow->SetText(_texts, nullptr, _compositionArgs->Selected);
}

void StrokeEditor::MelodyEditorPage::InitializePage()
{
	((App^)App::Current)->WriteToDebugFile("Инициализация страницы");

	// подгоняем ширину TextRow
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width - mySplitView->CompactPaneLength;
	_textRow->Width = width;

	moveSym = ref new SketchMusic::Commands::Handler([=](Object^ args) -> void
	{
		SketchMusic::Commands::SymbolHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return;

		//_text->moveSymbol(symArgs->_oldSym, symArgs->_newSym->_pos);
		symArgs->_oldSym->_pos = symArgs->_newSym->_pos;
	});

	addSym = ref new SketchMusic::Commands::Handler([=](Object^ args) -> void
	{
		SketchMusic::Commands::SymbolHandlerArgs^ symArgs = dynamic_cast<SketchMusic::Commands::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return;

		auto type = symArgs->_newSym->_sym->GetSymType();
		if (type == SymbolType::TEMPO || type == SymbolType::NPART || type == SymbolType::CLEF)
		{
			// темп и может какие-нибудь другие символы будем заменять вместо добавления новых в ту же точку
			symArgs->_text->addOrReplaceSymbol(symArgs->_newSym);
		}
		else symArgs->_text->addSymbol(symArgs->_newSym);
		
		
		// добавить внешнее представление символа
		_textRow->AddSymbol(symArgs->_text, symArgs->_newSym);
		if (viewType == ViewType::ChordView)
		{
			if (symArgs->_newSym->_sym->GetSymType() == SymbolType::NOTE) this->CurrentChord->GetNotes()->Append(symArgs->_newSym);
			else this->CurrentGChord->GetNotes()->Append(symArgs->_newSym);
		}
	});

	deleteSym = ref new SketchMusic::Commands::Handler([=](Object^ args) -> void
	{
		_textRow->Backspace();
	});
	
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
	//bpmChangeToken = 
	//	((App^)App::Current)->_player->BpmChanged += ref new Windows::Foundation::EventHandler<float>(this, &StrokeEditor::MelodyEditorPage::OnBpmChanged);
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

	// сохраняем данные в виде текста
	if (_idea)
	{
		_idea->SerializedContent = _idea->Content->serialize()->Stringify();
		this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(_idea, true));
	}

	if (_compositionArgs)
	{
		this->Frame->Navigate(TypeName(StrokeEditor::CompositionEditorPage::typeid), _compositionArgs);
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
							((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
								ref new SMC::Command(addSym, nullptr, nullptr),
								ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
									ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym))));
							((App^)App::Current)->_manager->ExecuteLast();
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

			//if (recording)
			//{
			// создаём команду на добавление ноты в текст и сохраняем её в истории
			((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(addSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_texts->ControlText, nullptr,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym))));
			((App^)App::Current)->_manager->ExecuteLast();
			//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
			//	+ " / tick = " + _textRow->currentPosition->getTick();
			//}
			break;
		}
		case SketchMusic::View::KeyType::enter:
			// перенос строки обрабатываем на отпускании
			break;
		case SketchMusic::View::KeyType::move:
		{
			if (args->key->value >= 0)
			{
				if (viewType == ViewType::TextRow) _textRow->MoveCursorRight();
				else MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
				//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				//	+ " / tick = " + _textRow->currentPosition->getTick();
			}
			else
			{
				if (viewType == ViewType::TextRow) _textRow->MoveCursorLeft();
				else MoveLeftCWBtn_Click(this, ref new RoutedEventArgs);
				//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				//	+ " / tick = " + _textRow->currentPosition->getTick();
			}
			break;
		}
		case SketchMusic::View::KeyType::record:
			recording = args->key->value;
			((App^)App::Current)->_player->recording = recording;
			break;
		case SketchMusic::View::KeyType::deleteSym:
		{
			// из текущего текста
			((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(deleteSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_textRow->current,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), nullptr),
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition->Beat - 1), nullptr))));
			((App^)App::Current)->_manager->ExecuteLast();

			// из управляющего текста - из управляющего текста удалять только через контекстное меню конкретного символа?
			// ! Это не требуется, т.к. сейчас удаление символа происходит через Backspace, а он сам учитывает всё что надо
			//((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
			//	ref new SMC::Command(deleteSym, nullptr, nullptr),
			//	ref new SMC::SymbolHandlerArgs(_texts->ControlText,
			//		ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), nullptr),
			//		ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition->getBeat() - 1), nullptr))));
			//((App^)App::Current)->_manager->ExecuteLast();
			//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
			//	+ " / tick = " + _textRow->currentPosition->getTick();
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
		// создаём команду на добавление ноты в текст и сохраняем её в истории
		((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
			ref new SMC::Command(addSym, nullptr, nullptr),
			ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
				ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), ref new SNewLine))));
		((App^)App::Current)->_manager->ExecuteLast();
		break;
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
			break;
		case SketchMusic::Player::PlayerState::STOP:
		case SketchMusic::Player::PlayerState::WAIT:
			this->_keyboard->OnKeyboardStateChanged(this, ref new SketchMusic::View::KeyboardState(SketchMusic::View::KeyboardStateEnum::stop));
			break;
		}
	}));
}


void StrokeEditor::MelodyEditorPage::OnBpmChanged(Platform::Object ^sender, float args)
{
	//this->Dispatcher->RunAsync(
	//	Windows::UI::Core::CoreDispatcherPriority::Normal,
	//	ref new Windows::UI::Core::DispatchedHandler([=]() {
	//	this->BPMText->Text = "" + ((App^)App::Current)->_player->_BPM;
	//}));
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
		_textRow->SetText(_texts, text, _compositionArgs ? _compositionArgs->Selected : -1);
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
								this->_texts->texts->Append(ref new Text(ref new Instrument(instr->_name, preset->name)));
								list->SelectedItem = nullptr;
							}
						}
					});
				}));
			});
		}));
	}
	InstrumentsFlyout->Hide();
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
			_textRow->SetText(_texts, _texts->texts->GetAt(new_index), _compositionArgs->Selected);
		}
		DeleteTextFlyout->Hide();
	}
}


void StrokeEditor::MelodyEditorPage::menu_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	if (HamburgerButton == (ContentControl^)e->ClickedItem)
	{
		bool isOpen = !mySplitView->IsPaneOpen;
		mySplitView->IsPaneOpen = isOpen;
		MenuSeparator->X2 = (isOpen ? 22. : menu->Width);
		MenuSeparator2->X2 = (isOpen ? 22. : menu->Width);
	}
	else if (homeItem == (ContentControl^)e->ClickedItem)
	{
		GoBackBtn_Click(this, nullptr);
	}
	else if (textsItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(textsCtrl);
	}
	else if (partsItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(partsCtrl);
	}
	else if (addItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(addCtrl);
	}
	else if (deleteItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(deleteCtrl);
	}
	else if (ChangeViewItem == (ContentControl^)e->ClickedItem)
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
	else if (settingsItem == (ContentControl^)e->ClickedItem)
	{
		//myFrame->Navigate(settingsItem->GetType());
	}
}


void StrokeEditor::MelodyEditorPage::_textRow_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	
}


void StrokeEditor::MelodyEditorPage::Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width
		- mySplitView->CompactPaneLength - _textRow->Margin.Left - _textRow->Margin.Right;
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
	CurrentGChord->SetNotes(txt->getNotesAtExcluding(pos, SymbolType::NOTE));

	auto prevPos = txt->getPosAtLeft(pos);
	PrevChord->SetNotes(txt->getNotesAt(prevPos, SymbolType::NOTE));
	PrevGChord->SetNotes(txt->getNotesAtExcluding(prevPos, SymbolType::NOTE));

	bool prevEnabled = (PrevChord->GetNotes() == nullptr) ? false : (PrevChord->GetNotes()->Size > 0);
	if (prevEnabled == false) prevEnabled = (PrevGChord->GetNotes() == nullptr) ? false : (PrevGChord->GetNotes()->Size > 0);
	
	if (prevEnabled)
	{
		MoveLeftCWBtn->IsEnabled = true; PrevPosTxt->Text = L"" + prevPos->Beat + ":" + prevPos->Tick ;
	}
	else { MoveLeftCWBtn->IsEnabled = false; PrevPosTxt->Text = ""; }

	auto nextPos = txt->getPosAtRight(pos);
	NextChord->SetNotes(_textRow->current->getNotesAt(nextPos, SymbolType::NOTE));
	NextGChord->SetNotes(_textRow->current->getNotesAtExcluding(nextPos, SymbolType::NOTE));
	
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

}
