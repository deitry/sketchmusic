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

namespace
{
	const double collapsedWidth = 22.;
}

using namespace StrokeEditor;
using namespace SketchMusic;
namespace SM = SketchMusic;
namespace SMV = SketchMusic::View;
namespace SMC = SketchMusic::Commands;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Core;
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

void MelodyEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
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

	ApplicationDataContainer^ localSettings =
		::ApplicationData::Current->LocalSettings;

	if (localSettings)
	{
		if (localSettings->Values->HasKey("need_metronome"))
		{
			((App^)App::Current)->_player->needMetronome = (bool)localSettings->Values->Lookup("need_metronome");
			_keyboard->SetKey(SMV::KeyType::metronome, ((App^)App::Current)->_player->needMetronome);
		}
		else 
		{
			((App^)App::Current)->_player->needMetronome = true;
		}

		if (localSettings->Values->HasKey("need_play_generic"))
		{
			((App^)App::Current)->_player->needPlayGeneric 
					= (bool)localSettings->Values->Lookup("need_play_generic");

			_keyboard->SetKey(SMV::KeyType::playGeneric, 
							  ((App^)App::Current)->_player->needPlayGeneric);
		}
		else {
			((App^)App::Current)->_player->needPlayGeneric = true;
		}

		if (localSettings->Values->HasKey("precount"))
		{
			Object^ obj = localSettings->Values->Lookup("precount");
			bool has = (obj) ? true
							 : false;
			
			// не знаю, почему с ним так сложно.
			// но почему-то тут периодически всё падает
			
			((App^)App::Current)->_player->precount = has ? 4 : 0;
			_keyboard->SetKey(SMV::KeyType::precount, 
							  ((App^)App::Current)->_player->precount);
		}
		else 
		{
			((App^)App::Current)->_player->precount = 4;
		}
	}

	((App^)App::Current)->_player->StopAtLast = false;

	((App^)App::Current)->WriteToDebugFile("Загрузка настроек завершена. Почти всё");

	TextsList->DataContext = _texts->texts;
	TextsList->SelectedIndex = 0;

	PartsList->DataContext = _texts->getParts();
	
	viewType = SMV::ViewType::TextRow;

	((App^)App::Current)->WriteToDebugFile("Инициализация страницы завершена");

	_keyboard->Focus(Xaml::FocusState::Programmatic);
}

void MelodyEditorPage::LoadIdea()
{
	((App^)App::Current)->WriteToDebugFile("Загрузка данных идеи");

	if (_idea->Content == nullptr)
	{
		// восстанавливаем из сериализованного состояния
		if (_idea->SerializedContent != nullptr)
		{
			_idea->Content = CompositionData::deserialize(_idea->SerializedContent);
		}
		else
		{
			_idea->Content = ref new CompositionData();
			
			::ApplicationDataContainer^ localSettings =
				::ApplicationData::Current->LocalSettings;

			if (localSettings->Values->HasKey("default_instr"))
			{
				auto str = (String^)localSettings->Values->Lookup("default_instr");
				JsonObject^ json = ref new JsonObject;
				if (JsonObject::TryParse(str, &json))
				{
					auto instr = Instrument::Deserialize(json);
					if (instr)
						_idea->Content->texts->Append(ref new SM::Text(instr));
				}
			}

			// если ключа не было или не смогли восстановить инструмент
			if (_idea->Content->texts->Size == 0)
			{
				_idea->Content->texts->Append(ref new SM::Text(ref new Instrument("grand_piano.sf2")));
			}
		}
	}

	_texts = _idea->Content;
	_textRow->SetText(_texts, nullptr);
}

void MelodyEditorPage::LoadComposition()
{
	((App^)App::Current)->WriteToDebugFile("Загрузка данных композиции");

	// полагаем, что у композиции данные уже существуют
	if (_compositionArgs->Project->Data->texts->Size == 0)
	{
		ApplicationDataContainer^ localSettings =
			ApplicationData::Current->LocalSettings;
		
		if (localSettings->Values->HasKey("default_instr"))
		{
			auto str = (String^)localSettings->Values->Lookup("default_instr");
			JsonObject^ json = ref new JsonObject;
			if (JsonObject::TryParse(str, &json))
			{
				auto instr = Instrument::Deserialize(json);
				if (instr)
					_compositionArgs->Project->Data->texts->Append(ref new SM::Text(instr));
			}
		}

		if (_compositionArgs->Project->Data->texts->Size == 0)
		{
			_compositionArgs->Project->Data->texts->Append(ref new SM::Text(ref new Instrument("grand_piano.sf2")));
		}
	}

	_texts = _compositionArgs->Project->Data;
	_textRow->SetText(_texts, nullptr, _compositionArgs->Selected);

	partsItem->Visibility = Xaml::Visibility::Visible;
}

void MelodyEditorPage::InitializePage()
{
	((App^)App::Current)->WriteToDebugFile("Инициализация страницы");

	// подгоняем ширину TextRow
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width 
				- MenuSplitView->CompactPaneLength;
	_textRow->Width = width;

	// Обработчики для команд
	AddSymHandler = ref new SMC::Handler([=] (Object^ args) -> bool 
	{
		SMC::SymbolHandlerArgs^ symArgs = dynamic_cast<SMC::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		AddSymbolToText(symArgs->_text, symArgs->_sym);
		
		return true;
	});

	DeleteSymHandler = ref new SMC::Handler([=] (Object^ args) -> bool 
	{
		SMC::SymbolHandlerArgs^ symArgs = dynamic_cast<SMC::SymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;
		
		DeleteSymbolFromText(symArgs->_text, symArgs->_sym);
		return true;
	});

	MoveSymHandler = ref new SMC::Handler([=] (Object^ args) -> bool 
	{
		SMC::SymbolPairHandlerArgs^ symArgs = dynamic_cast<SMC::SymbolPairHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// - изменяем положение символа
		symArgs->_text->moveSymbol(symArgs->_oldSym, symArgs->_newSym->_pos);
		// - меняем положение графического объекта
		_textRow->SetSymbolView(symArgs->_oldSym, symArgs->_newSym);
		return true;
	});
	
	ReverseMoveSymHandler = ref new SMC::Handler([=] (Object^ args) -> bool 
	{
		SMC::SymbolPairHandlerArgs^ symArgs = dynamic_cast<SMC::SymbolPairHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// - изменяем положение символа
		symArgs->_text->moveSymbol(symArgs->_newSym, symArgs->_oldSym->_pos);
		// - меняем положение графического объекта
		_textRow->SetSymbolView(symArgs->_newSym, symArgs->_oldSym);
		return true;
	});

	AddMultipleSymHandler = ref new SMC::Handler([=](Object^ args) -> bool 
	{
		SMC::MultiSymbolHandlerArgs^ symArgs = dynamic_cast<SMC::MultiSymbolHandlerArgs^>(args);
		if (symArgs == nullptr) return false;

		// проходимся по списку и добавляем каждый символ
		for (auto&& sym : symArgs->_OldSymbols)
		{
			AddSymbolToText(symArgs->_text, sym);
		}
		return true;
	});

	DeleteMultipleSymHandler = ref new SMC::Handler([=] (Object^ args) -> bool {
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

	(safe_cast<::SMV::BaseKeyboard^>(this->_keyboard))->KeyPressed 
		+= ref new ::EventHandler<::SMV::KeyboardEventArgs^>
						(this, (void(MelodyEditorPage::*) (::Platform::Object^, ::SMV::KeyboardEventArgs^)) &MelodyEditorPage::_keyboard_KeyboardPressed);

	(safe_cast<::SMV::BaseKeyboard^>(this->_keyboard))->KeyReleased 
		+= ref new ::EventHandler<::SMV::KeyboardEventArgs^>
						(this, (void(MelodyEditorPage::*) (::Platform::Object^, ::SMV::KeyboardEventArgs^)) &MelodyEditorPage::_keyboard_KeyReleased);
	
	curPosChangeToken = 
		((App^)App::Current)->_player->CursorPosChanged 
			+= ref new EventHandler<SM::Cursor ^>(this, &MelodyEditorPage::OnCursorPosChanged);
	
	playerStateChangeToken =
		((App^)App::Current)->_player->StateChanged 
			+= ref new EventHandler<SM::Player::PlayerState>
												(this, &MelodyEditorPage::OnStateChanged);

	KeyDownToken =
		Window::Current->CoreWindow->KeyDown 
			+= ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>
												(this, &MelodyEditorPage::OnKeyDown);

	KeyUpToken =
		Window::Current->CoreWindow->KeyUp 
			+= ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>
												(this, &MelodyEditorPage::OnKeyUp);
	
	//bpmChangeToken = 
	//	((App^)App::Current)->_player->BpmChanged += ref new EventHandler<float>(this, &MelodyEditorPage::OnBpmChanged);

	// привязка состояния
	canUndoChangeToken =
		((App^)App::Current)->_manager->CanUndoChanged 
				+= ref new EventHandler<bool>(this, &MelodyEditorPage::OnCanUndoChanged);

	canRedoChangeToken =
		((App^)App::Current)->_manager->CanRedoChanged 
				+= ref new EventHandler<bool>(this, &MelodyEditorPage::OnCanRedoChanged);
	
	// подписка на событие нажатия ноты, чтобы её озвучить
	_textRow->SymbolPressed 
		+= ref new EventHandler<SketchMusic::PositionedSymbol ^>(
												this, 
												&MelodyEditorPage::OnSymbolPressed);
}


void MelodyEditorPage::GoBackBtn_Click(Platform::Object^ sender, 
									   RoutedEventArgs^ e)
{
	// остановить проигрывание, если идёт
	((App^)App::Current)->_player->cycling = false;
	((App^)App::Current)->_player->stop();
	((App^)App::Current)->_player->stopKeyboard();
	
	// сохраняем настройки
	ApplicationDataContainer^ localSettings =
		ApplicationData::Current->LocalSettings;

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
		this->Frame->Navigate(TypeName(CompositionEditorPage::typeid), 
							  _compositionArgs);
	}
	else if (_idea)
	{	
		this->Frame->Navigate(TypeName(LibraryEntryPage::typeid), 
							  ref new LibraryEntryNavigationArgs(_idea, true));
	}
}


void MelodyEditorPage::SaveData()
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

void MelodyEditorPage::playAll_Click()
{
	if (((App^)App::Current)->_player->_state != SM::Player::PlayerState::STOP)
	{
		((App^)App::Current)->_player->stop();
		((App^)App::Current)->WriteToDebugFile("Остановка плеера");
		return;
	}

	this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
							   ref new DispatchedHandler([this] ()
	{
		auto async = concurrency::create_task([this]
		{
			//((App^)App::Current)->_player->stop();
			((App^)App::Current)->WriteToDebugFile("Запуск плеера");
			((App^)App::Current)->_player->playText(this->_texts, 
													ref new SM::Cursor(_textRow->currentPosition));
		});
	}));
}

void MelodyEditorPage::_keyboard_KeyboardPressed(Platform::Object^ sender, 
												 SMV::KeyboardEventArgs^ args)
{
	if (_textRow->current == nullptr) return;

	if (args->key)
	{
		switch (args->key->type)
		{
			case SMV::KeyType::metronome:
			{
				((App^)App::Current)->_player->needMetronome = (bool)args->key->value;
				ApplicationData::Current->LocalSettings->Values->Insert(
														"need_metronome", 
														((App^)App::Current)->_player->needMetronome);
				break;
			}
			case SMV::KeyType::playGeneric:
			{
				((App^)App::Current)->_player->needPlayGeneric = (bool)args->key->value;
				ApplicationData::Current->LocalSettings->Values->Insert(
														"need_play_generic", 
														((App^)App::Current)->_player->needMetronome);
				break;
			}
			case SMV::KeyType::quantization:
			{
				((App^)App::Current)->_player->quantize = args->key->value;
				_textRow->quantize = args->key->value;
				break;
			}
			case SMV::KeyType::precount:
			{
				((App^)App::Current)->_player->precount = args->key->value;
				ApplicationData::Current->LocalSettings->Values->Insert(
												"precount", 
												((App^)App::Current)->_player->needMetronome);
				break;
			}
			case SMV::KeyType::end:
			case SMV::KeyType::note:
			case SMV::KeyType::relativeNote:
			case SMV::KeyType::genericNote:
			{
				SM::ISymbol^ sym = SM::ISymbolFactory::CreateSymbol(args->key->type, 
																	args->key->value);

				auto snote = dynamic_cast<SM::SNote^>(sym);
				if (snote)
				{
					SM::Player::NoteOff^ noteOff = ref new SM::Player::NoteOff;
					((App^)App::Current)->_player->playSingleNote(snote, _textRow->current->instrument, 
																  0, noteOff);

					// сохраняем noteOff где-нибудь, ассоциированно с нажатой клавишей
					notesPlayingMap.insert(std::make_pair(args->key, noteOff));
				}

				// надо добавлять сразу аккордами
				if (recording)
				{
					//Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->
					Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
										 ref new DispatchedHandler([=]
					{
						auto delay = concurrency::create_task([=]
						{
							// небольшая задержка, чтобы можно было нажимать чуть-чуть заранее
							concurrency::wait(20);

							Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
												 ref new DispatchedHandler(
								[=] ()
							{
								// создаём команду на добавление ноты в текст и сохраняем её в истории
								((App^)App::Current)->_manager->AddAndExecute(
									AddSymCommand,
									ref new SMC::SymbolHandlerArgs(
										_textRow->current,
										ref new PositionedSymbol(
											ref new SM::Cursor(((App^)App::Current)->_player->quantize 
																			? _textRow->currentPosition 
																			: ((App^)App::Current)->_player->_cursor), 
											sym)));
							}));
						});
					}));
					//this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
					//	+ " / tick = " + _textRow->currentPosition->getTick();

					if (!appending 
						&& ((App^)App::Current)->_player->_state != SM::Player::PlayerState::PLAY)
					{
						appending = true;
						//Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->
							Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
												 ref new DispatchedHandler(
							[=] ()
						{
							// TODO: вынести в отдельную функцайку
							auto delay = concurrency::create_task([this]
							{
								unsigned int timeout = static_cast<unsigned int>(
														600 * 60 
														/ ((App^)App::Current)->_player->_BPM 
														/ _textRow->scale);
								concurrency::wait(timeout);

								Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
													 ref new DispatchedHandler([=] 
								{
									this->appending = false;
									if (this->viewType == SMV::ViewType::TextRow)
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
			// символы, добавляемые в "управляющий" текст 
			case SMV::KeyType::scale:
			case SMV::KeyType::harmony:
			case SMV::KeyType::tempo:
			{
				// создаём новый символ
				SM::ISymbol^ sym = SM::ISymbolFactory::CreateSymbol(args->key->type,
																	args->key->value);

				// создаём команду на добавление ноты в текст и сохраняем её в истории
				((App^)App::Current)->_manager->AddAndExecute(
					AddSymCommand,
					ref new SMC::SymbolHandlerArgs(
						_texts->ControlText, 
						ref new PositionedSymbol(ref new SM::Cursor(_textRow->currentPosition), 
												 sym)));

				if (args->key->type != SMV::KeyType::tempo)
				{
					// сдвигаем курсор для всех "управляющих" символов, кроме темпа (он вставляется по-другому)
					if (this->viewType == SMV::ViewType::TextRow)
					{
						this->_textRow->MoveCursorRight();
					}
					else
					{
						this->MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
					}
				}
				break;
			}
			case SMV::KeyType::enter:
				// перенос строки обрабатываем на отпускании
				break;
			case SMV::KeyType::move:
			{
				Dispatcher->RunAsync(CoreDispatcherPriority::Normal, 
									 ref new DispatchedHandler([=] 
				{
					if (args->key->value >= 0)
					{
						if (viewType == SMV::ViewType::TextRow)
							_textRow->MoveCursorRight();
						else MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
					}
					else
					{
						if (viewType == SMV::ViewType::TextRow)
							_textRow->MoveCursorLeft();
						else MoveLeftCWBtn_Click(this, ref new RoutedEventArgs);
					}
				}));
				break;
			}
			case SMV::KeyType::beat:
			{
				Dispatcher->RunAsync(CoreDispatcherPriority::Normal,
									 ref new DispatchedHandler([=]
				{
					// TODO: через команды, чтобы можно было отменять/возвращать

					if (args->key->value >= 0)
					{
						// добавить долю вперёд
						_texts->ControlText->addBeat(_textRow->currentPosition, ref new SM::Cursor(1));
						for (SM::Text^ text : _texts->texts)
						{
							text->addBeat(_textRow->currentPosition, ref new SM::Cursor(1));
						}

						// FIXME: сейчас - для увеличения размера
						_textRow->AddBeatItem_Click(this, ref new RoutedEventArgs());

						// перемещаем курсор
						if (viewType == SMV::ViewType::TextRow)
							_textRow->MoveCursorRight();
						else MoveRightCWBtn_Click(this, ref new RoutedEventArgs);
					}
					else
					{
						// удалить долю
						_texts->ControlText->deleteBeat(_textRow->currentPosition, ref new SM::Cursor(1));
						for (SM::Text^ text : _texts->texts)
						{
							text->deleteBeat(_textRow->currentPosition, ref new SM::Cursor(1));
						}

						// FIXME: сейчас - для уменьшения размера
						_textRow->DeleteBeatItem_Click(this, ref new RoutedEventArgs());

						// перемещаем курсор
						if (viewType == SMV::ViewType::TextRow)
							_textRow->MoveCursorLeft();
						else MoveLeftCWBtn_Click(this, ref new RoutedEventArgs);
					}
					_textRow->UpdateText();
				}));
				break;
			}
			case SMV::KeyType::record:
				recording = args->key->value;
				((App^)App::Current)->_player->recording = recording;
				break;
			case SMV::KeyType::backspace:
			{
				Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] 
				{
					Backspace();
				}));
				break;
			}
			case SMV::KeyType::stop:
				((App^)App::Current)->_player->stop();
				((App^)App::Current)->_player->stopKeyboard();
				break;
			case SMV::KeyType::play:
				playAll_Click();
				break;
			case SMV::KeyType::cycling:
				((App^)App::Current)->_player->cycling = args->key->value;
				((App^)App::Current)->_player->StopAtLast = ((App^)App::Current)->_player->cycling;
				break;
			case SMV::KeyType::zoom:
				_textRow->SetScale(args->key->value * 2);
				break;
			case SMV::KeyType::hide:
				break;
			case SMV::KeyType::eraser:
				_textRow->EraserTool = args->key->value;
				break;
			case SMV::KeyType::control:
				ctrlPressed = !ctrlPressed;
				break;
			case SMV::KeyType::octave:
			case SMV::KeyType::space:
			default:
				break;
		}
	}
}

void MelodyEditorPage::_keyboard_KeyReleased(Platform::Object^ sender, 
											 SMV::KeyboardEventArgs^ args)
{
	// на отпускание клавиши мы должны
	// - прекращать звучание
	for (auto noteIter = notesPlayingMap.begin(); noteIter != notesPlayingMap.end(); noteIter++)
	{
		if (((*noteIter).first->type == args->key->type) 
		 && ((*noteIter).first->value == args->key->value))
		{
			(*noteIter).second->Cancel();
			notesPlayingMap.erase(noteIter);
			break;
		}
	}

	// обрабатывание событий отпускания некоторых клавиш
	switch (args->key->type)
	{
		case SMV::KeyType::enter:
		{
			int beat = _textRow->currentPosition->Beat;
			if (_textRow->currentPosition->Tick > 0) beat++;
			// создаём команду на добавление ноты в текст и сохраняем её в истории
			auto newpos = ref new SM::Cursor(beat);
			((App^)App::Current)->_manager->AddAndExecute(
				AddSymCommand,
				ref new SMC::SymbolHandlerArgs(_texts->ControlText,
					ref new PositionedSymbol(newpos, ref new SNewLine)));
			_textRow->SetCursor(newpos);
			break;
		}
	}
}

void MelodyEditorPage::OnStateChanged(Platform::Object ^sender, 
									  SM::Player::PlayerState args)
{
	this->Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([=]() {
		((App^)App::Current)->WriteToDebugFile("Обработка изменения состояния плеера");
		switch (args)
		{
		case SM::Player::PlayerState::PLAY:
			this->_keyboard->OnKeyboardStateChanged(this, 
													ref new SMV::KeyboardState(
														SMV::KeyboardStateEnum::play));
			PlayItemTxt1->Text = L"\uE769";
			PlayItemTxt2->Text = L"Остановить";
			break;
		case SM::Player::PlayerState::STOP:
		case SM::Player::PlayerState::WAIT:
			this->_keyboard->OnKeyboardStateChanged(this, 
													ref new SMV::KeyboardState(
																SMV::KeyboardStateEnum::stop));
			PlayItemTxt1->Text = L"\uE768";
			PlayItemTxt2->Text = L"Воспроизвести";
			break;
		}
	}));
}

void MelodyEditorPage::OnCursorPosChanged(Platform::Object ^sender, 
										  SM::Cursor^ pos)
{
	create_task(this->Dispatcher->RunAsync(
		CoreDispatcherPriority::Low,
		ref new DispatchedHandler(
			[=]() 
	{
		_textRow->SetCursor(pos);
	})));
}

void MelodyEditorPage::ListView_SelectionChanged(Platform::Object^ sender, 
												 SelectionChangedEventArgs^ e)
{
	SM::Text^ text = dynamic_cast<SM::Text^>(TextsList->SelectedItem); // e->AddedItems->First()->Current
	if (text)
	{
		if (text != _textRow->current)
		{
			_textRow->SetText(_texts, text, 
							  _compositionArgs ? _compositionArgs->Selected 
											   : -1);
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
						CoreDispatcherPriority::Normal,
						ref new DispatchedHandler([=] () {

						ApplicationDataContainer^ localSettings =
							ApplicationData::Current->LocalSettings;

						this->_texts->texts->Append(ref new SM::Text(dialog->Selected));
					}));
				}
			});
		}
	}
	TextsFlyout->Hide();
}

void MelodyEditorPage::ListView_ItemClick(Platform::Object^ sender, 
										  ItemClickEventArgs^ e)
{
	auto instr = dynamic_cast<Instrument^>(e->ClickedItem);
	if (instr)
	{
		// спрашиваем у плеера, загружен ли такой инструмент и есть ли у него пресеты
		// - выводим окошко со списком доступных пресетов
		this->Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([=]() {
			
			auto init = concurrency::create_task([=]
			{
				// такая обёртка, если асинхронно будем читать данные, если они ешё не созданы
				return ((App^)App::Current)->_player->GetSFData(instr);
			})
				.then([=] (SM::SFReader::SFData^ data)->void 
			{
				this->Dispatcher->RunAsync(
					CoreDispatcherPriority::Normal,
					ref new DispatchedHandler([=]() {
				
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
					list->ItemClick += ref new ItemClickEventHandler([=] (Object^, Object^)
					{
						presetListDlg->Hide();
					});
					Button^ btn = ref new Button;
					btn->Content = "Отмена";
					btn->Click += ref new RoutedEventHandler([=] (Object^, Object^)
					{
						presetListDlg->Hide();
					});

					grid->Children->Append(list);
					grid->Children->Append(btn);
					btn->SetValue(Grid::ColumnProperty, 1);

					presetListDlg->Content = grid;

					auto dialog = concurrency::create_task(presetListDlg->ShowAsync());
					dialog.then([=] (concurrency::task<ContentDialogResult> t)
					{
						ContentDialogResult result = t.get();

						if (list->SelectedItem)// result == ContentDialogResult::Primary)
						{
							auto preset = dynamic_cast<SM::SFReader::SFPreset^>(list->SelectedItem);
							if (preset)
							{
								this->_texts->texts->Append(ref new SM::Text(ref new Instrument(instr->_name, 
																							instr->FileName, 
																							preset->name)));
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


void MelodyEditorPage::DeleteTextBtn_Click(Platform::Object^ sender, 
										   RoutedEventArgs^ e)
{
	auto list = dynamic_cast<ListView^>(TextsFlyout->Content);
	if (list)
	{
		SM::Text^ text = dynamic_cast<SM::Text^>(list->SelectedItem);
		auto size = _texts->texts->Size;
		if (text && (size > 1)) // последний удалить нельзя
		{
			unsigned int index;
			_texts->texts->IndexOf(text, &index);
			unsigned int new_index = index;
			
			if (new_index > (size - 2)) 
			{ 
				// size-1 = крайний элемент; size-1-1 = крайний элемент после удаления
				new_index = size - 2;
			}	
			TextsList->SelectedIndex = new_index;
			_texts->texts->RemoveAt(index);
			_textRow->SetText(_texts, _texts->texts->GetAt(new_index), 
							  (_compositionArgs) 
									?_compositionArgs->Selected 
									: -1);
		}
		DeleteTextFlyout->Hide();
	}
}


void MelodyEditorPage::Page_SizeChanged(Platform::Object^ sender, 
										SizeChangedEventArgs^ e)
{
	using namespace Windows::UI::ViewManagement;
	auto width = ApplicationView::GetForCurrentView()->VisibleBounds.Width
				- MenuSplitView->CompactPaneLength 
				- _textRow->Margin.Left - _textRow->Margin.Right;
	_textRow->Width = width;
}


// переместить курсор к ближайшей левой ноте
void MelodyEditorPage::MoveLeftCWBtn_Click(Platform::Object^ sender, 
										   RoutedEventArgs^ e)
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
void MelodyEditorPage::MoveRightCWBtn_Click(Platform::Object^ sender, 
											RoutedEventArgs^ e)
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

void MelodyEditorPage::UpdateChordViews(Cursor ^ pos)
{
	if (this->viewType != SMV::ViewType::ChordView) 
		return;

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

	bool prevEnabled = (PrevChord->GetNotes() == nullptr) 
		? false 
		: (PrevChord->GetNotes()->Size > 0);
	
	if (prevEnabled == false)
	{
		prevEnabled = (PrevGChord->GetNotes() == nullptr)
			? false
			: (PrevGChord->GetNotes()->Size > 0);
	}
	
	if (prevEnabled)
	{
		MoveLeftCWBtn->IsEnabled = true; 
		PrevPosTxt->Text = L"" + prevPos->Beat + ":" + prevPos->Tick ;
	}
	else 
	{ 
		MoveLeftCWBtn->IsEnabled = false; 
		PrevPosTxt->Text = ""; 
	}

	auto nextPos = txt->getPosAtRight(pos);
	auto nextPos2 = _texts->ControlText->getPosAtRight(pos);	// TODO : можно сделать лучше
	if (nextPos2 && nextPos2->LT(nextPos)) 
		nextPos = nextPos2;

	NextChord->SetNotes(_textRow->current->getNotesAt(nextPos, SymbolType::NOTE));
	notes = txt->getNotesAtExcluding(nextPos, SymbolType::NOTE);
	
	// так сложно, потому что часть нот попадает в ControlText
	notes2 = _texts->ControlText->getNotesAtExcluding(nextPos, SymbolType::NOTE);
	for (auto&& note : notes2)
	{
		notes->Append(note);
	}
	NextGChord->SetNotes(notes);
	
	bool nextEnabled = (NextChord->GetNotes() == nullptr) 
						? false 
						: (NextChord->GetNotes()->Size > 0);
	
	if (nextEnabled == false)
	{
		nextEnabled = (NextGChord->GetNotes() == nullptr)
			? false
			: (NextGChord->GetNotes()->Size > 0);
	}

	if (nextEnabled)
	{
		MoveRightCWBtn->IsEnabled = true; 
		NextPosTxt->Text = L"" + nextPos->Beat + ":" + nextPos->Tick;
	}
	else 
	{ 
		MoveRightCWBtn->IsEnabled = false; 
		NextPosTxt->Text = ""; 
	}

}

void MelodyEditorPage::PartsList_SelectionChanged(Platform::Object^ sender, 
												  SelectionChangedEventArgs^ e)
{
	if (PartsList->SelectedItem)
	{
		_textRow->SetText(_texts, _textRow->current, PartsList->SelectedIndex);
		_compositionArgs->Selected = PartsList->SelectedIndex;
	}
}


void MelodyEditorPage::MenuButton_Click(Platform::Object^ sender, 
										RoutedEventArgs^ e)
{
	if ((HamburgerButton == (ContentControl^)e->OriginalSource))
	{
		bool isOpen = !MenuSplitView->IsPaneOpen;
		MenuSplitView->IsPaneOpen = isOpen;
		auto width = (isOpen ? menu->ActualWidth : ::collapsedWidth);
		MenuSeparator->X2 = width;
		MenuSeparator2->X2 = width;
		MenuSeparator3->X2 = width;
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
		Flyout::ShowAttachedFlyout(textsCtrl);
	}
	else if (partsItem == (ContentControl^)e->OriginalSource)
	{
		Flyout::ShowAttachedFlyout(partsCtrl);
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
					CoreDispatcherPriority::Normal,
					ref new DispatchedHandler([=]() 
				{
					ApplicationDataContainer^ localSettings =
						ApplicationData::Current->LocalSettings;

					this->_texts->texts->Append(ref new SM::Text(dialog->Selected));
				}));
			}
		});
	}
	else if (deleteItem == (ContentControl^)e->OriginalSource)
	{
		Flyout::ShowAttachedFlyout(deleteCtrl);
	}
	else if (PlayItem == (ContentControl^)e->OriginalSource)
	{
		playAll_Click();
	}
	else if (ChangeViewItem == (ContentControl^)e->OriginalSource)
	{
		// TODO : если типов представления нот будет больше, 
		// сделать через флайаут или контентдиалог
		viewType = (viewType == SMV::ViewType::TextRow)
									? SMV::ViewType::ChordView 
									: SMV::ViewType::TextRow;

		// Соответствующим образом меняем представление
		switch (viewType)
		{
			case SMV::ViewType::TextRow:
			{
				_textRow->Visibility = Xaml::Visibility::Visible;
				_ChordViewGrid->Visibility = Xaml::Visibility::Collapsed;
				break;
			}
			case SMV::ViewType::ChordView:
			{
				_textRow->Visibility = Xaml::Visibility::Collapsed;
				_ChordViewGrid->Visibility = Xaml::Visibility::Visible;
				UpdateChordViews(_textRow->currentPosition);
				break;
			}
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

void MelodyEditorPage::UndoItem_Click(Platform::Object^ sender, 
									  RoutedEventArgs^ e)
{
	((App^)App::Current)->_manager->Undo();
}


void MelodyEditorPage::RedoItem_Click(Platform::Object^ sender, 
									  RoutedEventArgs^ e)
{
	((App^)App::Current)->_manager->Redo();
}

// FIXME: здесь этого быть не должно
void MelodyEditorPage::AddSymbolToText(SM::Text ^ text, 
									   SM::PositionedSymbol ^ symbol)
{
	// добавляем символ в текст
	auto type = symbol->_sym->GetSymType();
	if (type == SymbolType::TEMPO 
	 || type == SymbolType::NPART 
	 || type == SymbolType::CLEF
	 || type == SymbolType::HARMONY
	 || type == SymbolType::SCALE)
	{
		// удаляем предыдущий графический символ
		_textRow->DeleteSymbolViews(symbol->_pos, type);

		// темп и может какие-нибудь другие символы будем 
		// заменять вместо добавления новых в ту же точку
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

void MelodyEditorPage::DeleteSymbolFromText(SM::Text ^ text, 
											SM::PositionedSymbol ^ symbol)
{
	// удаляем символ из текста
	text->deleteSymbol(symbol->_pos, symbol->_sym);

	// удаляем графическое представление
	_textRow->DeleteSymbolView(symbol);
	// TODO : удаление символа из ChordView
}

// Перемещение курсора назад и удаление всех символов, 
// попавших в диапазон между исходным и конечным положением курсора
void MelodyEditorPage::Backspace()
{
	// если в текущем положении есть разрыв строки, сперва удаляем его
	auto curSyms = _texts->ControlText->GetSymbols(_textRow->currentPosition, 
												   _textRow->currentPosition);
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


void MelodyEditorPage::OnCanRedoChanged(Platform::Object ^sender, 
										bool args)
{
	RedoItem->IsEnabled = args;
}


void MelodyEditorPage::OnCanUndoChanged(Platform::Object ^sender, 
										bool args)
{
	UndoItem->IsEnabled = args;
}


void MelodyEditorPage::OnKeyDown(CoreWindow ^sender, 
								 KeyEventArgs ^args)
{
	if (args->KeyStatus.WasKeyDown == true)
		return;

	using Windows::System::VirtualKey;
	bool isCtrl = (sender->GetForCurrentThread()->GetKeyState(VirtualKey::Control) 
						!= CoreVirtualKeyStates::None);
	
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


void MelodyEditorPage::OnKeyUp(CoreWindow ^sender, 
							   KeyEventArgs ^args)
{
	using Windows::System::VirtualKey;
	if (args->VirtualKey == VirtualKey::Control)
		ctrlPressed = false;
}

// Озвучиваем нажатый на textRow символ
void MelodyEditorPage::OnSymbolPressed(Platform::Object ^sender, 
									   SM::PositionedSymbol ^args)
{
	auto snote = dynamic_cast<SM::SNote^>(args->_sym);
	if (snote)
	{
		((App^)App::Current)->_player->playSingleNote(snote, 
													  _textRow->current->instrument, 
													  500, nullptr);
	}
}


void MelodyEditorPage::TextsList_ItemClick(Platform::Object^ sender, 
										   ItemClickEventArgs^ e)
{
	SketchMusic::Text^ text = dynamic_cast<SM::Text^>(e->ClickedItem); // TextsList->SelectedItem
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
						CoreDispatcherPriority::Normal,
						ref new DispatchedHandler([=]() 
					{
						ApplicationDataContainer^ localSettings =
							ApplicationData::Current->LocalSettings;

						_textRow->current->instrument = dialog->Selected;
						//TextsList->ItemsSource = _texts;
					}));
				}
			});
		}
	}
	TextsFlyout->Hide();
}


void MelodyEditorPage::TextsFlyout_Opened(Platform::Object^ sender, 
										  Platform::Object^ e)
{
	int sel = TextsList->SelectedIndex;
	TextsList->DataContext = nullptr;
	TextsList->DataContext = _texts->texts;
	TextsList->SelectedIndex = sel;
}
