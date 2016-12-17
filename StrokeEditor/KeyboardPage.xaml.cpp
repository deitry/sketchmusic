//
// MainPage.xaml.cpp
// Реализация класса MainPage.
//

#include "pch.h"
#include "KeyboardPage.xaml.h"
#include "concrt.h"


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

using namespace SketchMusic;
namespace SMC = SketchMusic::Commands;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

Text^ testText()
{
	Text^ text = ref new Text;

	text->instrument = ref new Instrument("grand_piano.sf2");

	/*text->addSymbol(ref new Cursor(0), ref new SNote(3));
	text->addSymbol(ref new Cursor(1), ref new SNote(3));
	
	text->addSymbol(ref new Cursor(3), ref new SNote(10));
	text->addSymbol(ref new Cursor(4), ref new SNote(10));
	
	text->addSymbol(ref new Cursor(6), ref new SNote(4));
	text->addSymbol(ref new Cursor(7,16), ref new SNote(6));
	text->addSymbol(ref new Cursor(8), ref new SNote(7));
	
	text->addSymbol(ref new Cursor(10), ref new SNote(5));
	text->addSymbol(ref new Cursor(10), ref new SNote(2));

	text->addSymbol(ref new Cursor(12), ref new SNewLine);
	text->addSymbol(ref new Cursor(13, 16), ref new SNote(1));
	text->addSymbol(ref new Cursor(14), ref new SNote(13));
	text->addSymbol(ref new Cursor(14,16), ref new SNote(10));
	text->addSymbol(ref new Cursor(15), ref new SNote(8));
	text->addSymbol(ref new Cursor(15,16), ref new SNote(14));
	text->addSymbol(ref new Cursor(15,24), ref new SNote(13));
	text->addSymbol(ref new Cursor(16), ref new SNote(9));
	text->addSymbol(ref new Cursor(19), ref new SNote(8));
	text->addSymbol(ref new Cursor(22), ref new SNote(9));
	
	//
	text->addSymbol(ref new Cursor(0), ref new SNote(-12));
	text->addSymbol(ref new Cursor(3), ref new SNote(-6));
	text->addSymbol(ref new Cursor(6), ref new SNote(-11));
	text->addSymbol(ref new Cursor(9), ref new SNote(-13));
	text->addSymbol(ref new Cursor(12), ref new SNote(-14));

	text->addSymbol(ref new Cursor(15), ref new SNote(-15));
	text->addSymbol(ref new Cursor(18), ref new SNote(-18));
	text->addSymbol(ref new Cursor(21), ref new SNote(-17));
	text->addSymbol(ref new Cursor(24), ref new SNote(-12));
	*/
	return text;
}

KeyboardPage::KeyboardPage()
{
	InitializeComponent();

	
	//VisualStateManager::GoToState(_position, "Normal", true);

	_manager = ref new SketchMusic::Commands::CommandManager;

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

	auto init = concurrency::create_task([this]
	{
		_player = ref new SketchMusic::Player::Player;
	});

	//_text = testText();
	_texts = ref new Platform::Collections::Vector < Text^ > ;

	_texts->Append(ref new Text("grand_piano.sf2"));
	_texts->Append(ref new Text("acoustic_guitar.sf2"));
	_texts->Append(ref new Text("Epiphone_Pick_Bass.sf2"));
	_texts->Append(ref new Text("Solo_Viola.sf2"));

	ListView^ list = dynamic_cast<ListView^>(TextsFlyout->Content);
	if (list)
	{
		list->DataContext = _texts;
	}

	_textRow->SetText(_texts, nullptr);

	(safe_cast<::SketchMusic::View::GenericKeyboard^>(this->_keyboard))->KeyPressed += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::KeyboardPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&KeyboardPage::_keyboard_KeyboardPressed);
	(safe_cast<::SketchMusic::View::GenericKeyboard^>(this->_keyboard))->KeyReleased += ref new ::Windows::Foundation::EventHandler<::SketchMusic::View::KeyboardEventArgs^>(this, (void(::StrokeEditor::KeyboardPage::*)
		(::Platform::Object^, ::SketchMusic::View::KeyboardEventArgs^))&KeyboardPage::_keyboard_KeyReleased);
}

void StrokeEditor::KeyboardPage::playAll_Click()
{
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		auto async = concurrency::create_task([this]
		{
			this->_player->stop();
			this->_player->playText(this->_texts, ref new SketchMusic::Cursor);
		});
	}));
}

void StrokeEditor::KeyboardPage::_keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ args)
{
	//SketchMusic::View::Key^ key = dynamic_cast<SketchMusic::View::Key^>(e);
	if (args->key)
	{
		switch (args->key->type)
		{
		case SketchMusic::View::KeyType::note:
		case SketchMusic::View::KeyType::relativeNote:
		case SketchMusic::View::KeyType::genericNote:
		{
			SketchMusic::ISymbol^ sym = ref new SketchMusic::SNote(args->key->value);

			SketchMusic::Player::NoteOff^ noteOff = ref new SketchMusic::Player::NoteOff;
			_player->playSingleNote((SketchMusic::SNote^) sym, _textRow->current->instrument, 0, noteOff);
			// сохраняем noteOff где-нибудь, ассоциированно с нажатой клавишей
			notesPlayingMap.insert(std::make_pair(args->key, noteOff));

			// надо добавлять сразу аккордами
			if (recording)
			{
				// создаём команду на добавление ноты в текст и сохраняем её в истории
				_manager->AddCommand(ref new SMC::CommandState(
					ref new SMC::Command(addSym, nullptr, nullptr),
					ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
					ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), sym))));
				_manager->ExecuteLast();
				this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
					+ " / tick = " + _textRow->currentPosition->getTick();

				if (!appending)
				{
					appending = true;
					//Window::Current
					//Windows::ApplicationModel::Core::CoreApplication::MainView
					//_textRow->_mainPanel
					Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]
					{
						auto delay = concurrency::create_task([this]
						{
							unsigned int timeout = 1000 * 60 / _player->_BPM / _textRow->scale;
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
		case SketchMusic::View::KeyType::enter:
			// создаём команду на добавление ноты в текст и сохраняем её в истории
			_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(addSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_textRow->current, nullptr,
				ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), ref new SNewLine))));
			_manager->ExecuteLast();
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
			_manager->AddCommand(ref new SMC::CommandState(
				ref new SMC::Command(deleteSym, nullptr, nullptr),
				ref new SMC::SymbolHandlerArgs(_textRow->current,
				ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition), nullptr),
				ref new PositionedSymbol(ref new SketchMusic::Cursor(_textRow->currentPosition->getBeat() - 1), nullptr))));
			_manager->ExecuteLast();
			this->CurPos->Text = "beat = " + _textRow->currentPosition->getBeat()
				+ " / tick = " + _textRow->currentPosition->getTick();
			break;
		}
		case SketchMusic::View::KeyType::stop:
			_player->stop();
			_player->stopKeyboard();
			break;
		case SketchMusic::View::KeyType::play:
			playAll_Click();
			break;
		case SketchMusic::View::KeyType::cycling:
			_player->cycling = args->key->value;
			break;
		case SketchMusic::View::KeyType::tempo:
			_player->_BPM = args->key->value;
			break;
		case SketchMusic::View::KeyType::zoom:
			_textRow->SetScale(args->key->value*2);
			break;
		case SketchMusic::View::KeyType::octave:
		case SketchMusic::View::KeyType::space:
		default:
			break;
		}
	}
}

void StrokeEditor::KeyboardPage::_keyboard_KeyReleased(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ args)
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

void StrokeEditor::KeyboardPage::ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	Text^ text = dynamic_cast<Text^>(e->AddedItems->First()->Current);
	if (text)
		_textRow->SetText(text);
}


void StrokeEditor::KeyboardPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//We should verify if there are pages in the navigation back stack 
	//before navigating to the previous page.
	if (this->Frame != nullptr && Frame->CanGoBack)
	{
		//Using the GoBack method, the frame navigates to the previous page.
		Frame->GoBack();
	}
}
