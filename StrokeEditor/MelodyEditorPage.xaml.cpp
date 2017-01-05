//
// MelodyEditorPage.xaml.cpp
// Реализация класса MelodyEditorPage
//

#include "pch.h"
#include "MelodyEditorPage.xaml.h"
#include "concrt.h"
#include "LibraryEntryPage.xaml.h"

using namespace StrokeEditor;
using namespace SketchMusic;
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
	InitializeComponent();
}

void StrokeEditor::MelodyEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	InitializePage();

	//((App^)App::Current)->ShowNotification("Загрузка страницы");
	
	_idea = (Idea^)e->Parameter;
	
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
		}
	}
	
	_texts = _idea->Content;
	
	//((App^)App::Current)->ShowNotification("Загрузка параметров");
	
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	if (localSettings)
	{
		if (localSettings->Values->HasKey("need_metronome"))
		{
			((App^)App::Current)->_player->needMetronome = (bool)localSettings->Values->Lookup("need_metronome");
		}
		else {
			((App^)App::Current)->_player->needMetronome = true;
		}
	}

	//((App^)App::Current)->ShowNotification("Загрузка инструментов");

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
	}
	
	_textRow->SetText(_texts, nullptr);
}

void StrokeEditor::MelodyEditorPage::InitializePage()
{
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

		_textRow->current->addSymbol(symArgs->_newSym);
		// добавить внешнее представление символа
		_textRow->AddSymbol(symArgs->_newSym);
	});

	deleteSym = ref new SketchMusic::Commands::Handler([=](Object^ args) -> void
	{
		_textRow->Backspace();
	});
	
	(safe_cast<::SketchMusic::View::GenericKeyboard^>(this->_keyboard))->KeyPressed += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::MelodyEditorPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&MelodyEditorPage::_keyboard_KeyboardPressed);
	(safe_cast<::SketchMusic::View::GenericKeyboard^>(this->_keyboard))->KeyReleased += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::MelodyEditorPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&MelodyEditorPage::_keyboard_KeyReleased);
	
	curPosChangeToken = 
		((App^)App::Current)->_player->CursorPosChanged += 
		ref new Windows::Foundation::EventHandler<SketchMusic::Cursor ^>(this, &StrokeEditor::MelodyEditorPage::OnCursorPosChanged);
	playerStateChangeToken =
		((App^)App::Current)->_player->StateChanged += 
		ref new Windows::Foundation::EventHandler<SketchMusic::Player::PlayerState>(this, &StrokeEditor::MelodyEditorPage::OnStateChanged);
	bpmChangeToken = 
		((App^)App::Current)->_player->BpmChanged += ref new Windows::Foundation::EventHandler<float>(this, &StrokeEditor::MelodyEditorPage::OnBpmChanged);
}


void StrokeEditor::MelodyEditorPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// остановить проигрывание, если идёт
	((App^)App::Current)->_player->cycling = false;
	((App^)App::Current)->_player->stop();
	((App^)App::Current)->_player->stopKeyboard();

	// сохраняем данные в виде текста
	_idea->SerializedContent = _idea->Content->serialize()->Stringify();
	
	// сохраняем настройки
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	localSettings->Values->Insert("metronome", ((App^)App::Current)->_player->needMetronome);

	// отписка от события
	(((App^)App::Current)->_player)->StateChanged -= playerStateChangeToken;

	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(_idea, true));
}

void StrokeEditor::MelodyEditorPage::playAll_Click()
{
	if (((App^)App::Current)->_player->_state != SketchMusic::Player::PlayerState::STOP)
	{
		((App^)App::Current)->_player->stop();
		return;
	}

	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		auto async = concurrency::create_task([this]
		{
			//((App^)App::Current)->_player->stop();
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
			break;
		}
		case SketchMusic::View::KeyType::note:
		case SketchMusic::View::KeyType::relativeNote:
		case SketchMusic::View::KeyType::genericNote:
		{
			SketchMusic::ISymbol^ sym = ref new SketchMusic::SNote(args->key->value);

			SketchMusic::Player::NoteOff^ noteOff = ref new SketchMusic::Player::NoteOff;
			((App^)App::Current)->_player->playSingleNote((SketchMusic::SNote^) sym, _textRow->current->instrument, 0, noteOff);
			// сохраняем noteOff где-нибудь, ассоциированно с нажатой клавишей
			notesPlayingMap.insert(std::make_pair(args->key, noteOff));

			// надо добавлять сразу аккордами
			if (recording)
			{
				// создаём команду на добавление ноты в текст и сохраняем её в истории
				((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
					ref new SMC::Command(addSym, nullptr, nullptr),
					ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
						ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym))));
				((App^)App::Current)->_manager->ExecuteLast();
				this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
					+ " / tick = " + _textRow->currentPosition->getTick();

				if (!appending)
				{
					appending = true;
					Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
					{
						auto delay = concurrency::create_task([this]
						{
							unsigned int timeout = 300 * 60 / ((App^)App::Current)->_player->_BPM / _textRow->scale;
							concurrency::wait(timeout);
							Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=] {
								this->appending = false;
								this->_textRow->MoveCursorRight();
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
				ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym))));
			((App^)App::Current)->_manager->ExecuteLast();
			this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				+ " / tick = " + _textRow->currentPosition->getTick();
			//}
			break;
		}
		case SketchMusic::View::KeyType::enter:
			// создаём команду на добавление ноты в текст и сохраняем её в истории
			((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(addSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), ref new SNewLine))));
			((App^)App::Current)->_manager->ExecuteLast();
			this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				+ " / tick = " + _textRow->currentPosition->getTick();
			break;
		case SketchMusic::View::KeyType::move:
		{
			if (args->key->value >= 0)
			{
				_textRow->MoveCursorRight();
				this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
					+ " / tick = " + _textRow->currentPosition->getTick();
			}
			else
			{
				_textRow->MoveCursorLeft();
				this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
					+ " / tick = " + _textRow->currentPosition->getTick();
			}
			break;
		}
		case SketchMusic::View::KeyType::record:
			recording = args->key->value;
			break;
		case SketchMusic::View::KeyType::deleteSym:
		{
			((App^)App::Current)->_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(deleteSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_textRow->current,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), nullptr),
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition->getBeat() - 1), nullptr))));
			((App^)App::Current)->_manager->ExecuteLast();
			this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				+ " / tick = " + _textRow->currentPosition->getTick();
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
			break;
		case SketchMusic::View::KeyType::zoom:
			_textRow->SetScale(args->key->value * 2);
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
}

void StrokeEditor::MelodyEditorPage::OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args)
{
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
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
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
		this->BPMText->Text = "" + ((App^)App::Current)->_player->_BPM;
	}));
}

void StrokeEditor::MelodyEditorPage::OnCursorPosChanged(Platform::Object ^sender, SketchMusic::Cursor^ pos)
{
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
			_textRow->SetCursor(pos);
			//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
			//	+ " / tick = " + _textRow->currentPosition->getTick();
	}));
}

void StrokeEditor::MelodyEditorPage::ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	Text^ text = dynamic_cast<Text^>(TextsList->SelectedItem); // e->AddedItems->First()->Current
	if (text)
	{
		_textRow->SetText(text);
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
			_textRow->SetText(_texts->texts->GetAt(new_index));
		}
		DeleteTextFlyout->Hide();
	}
}


void StrokeEditor::MelodyEditorPage::menu_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	if (HamburgerButton == (ContentControl^)e->ClickedItem)
	{
		mySplitView->IsPaneOpen = !mySplitView->IsPaneOpen;
	}
	else if (homeItem == (ContentControl^)e->ClickedItem)
	{
		GoBackBtn_Click(this, nullptr);
	}
	else if (textsItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(textsCtrl);
	}
	else if (addItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(addCtrl);
	}
	else if (deleteItem == (ContentControl^)e->ClickedItem)
	{
		Windows::UI::Xaml::Controls::Flyout::ShowAttachedFlyout(deleteCtrl);
	}
	else if (settingsItem == (ContentControl^)e->ClickedItem)
	{
		//myFrame->Navigate(settingsItem->GetType());
	}
}
