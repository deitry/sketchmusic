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

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

MelodyEditorPage::MelodyEditorPage()
{
	InitializeComponent();
}

void StrokeEditor::MelodyEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	InitializePage();

	_texts = ref new Platform::Collections::Vector < Text^ >;
	_idea = (Idea^)e->Parameter;
	
	if (_idea->Content == nullptr)
	{
		_idea->Content = ref new Text(ref new Instrument("grand_piano.sf2"));
		// восстанавливаем из сериализованного состояния
		if (_idea->SerializedContent != nullptr)
		{
			_idea->Content->deserialize(_idea->SerializedContent);
		}
	}
	else {
		// принудительно, пока нет выбора других инструментов
		_idea->Content->instrument = ref new Instrument("grand_piano.sf2");
	}
	_texts->Append(_idea->GetContent());

	//ListView^ list = dynamic_cast<ListView^>(TextsFlyout->Content);
	//if (list)
	//{
	//	list->DataContext = _texts;
	//}

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
}


void StrokeEditor::MelodyEditorPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// остановить проигрывание, если идёт
	((App^)App::Current)->_player->cycling = false;
	((App^)App::Current)->_player->stop();
	((App^)App::Current)->_player->stopKeyboard();

	// открыть страницу со сведениями о данной идее
	_idea->Content = this->_texts->GetAt(0);
	_idea->SerializedContent = _idea->Content->serialize()->Stringify();
	this->Frame->Navigate(TypeName(StrokeEditor::LibraryEntryPage::typeid), ref new LibraryEntryNavigationArgs(_idea, true));
}

void StrokeEditor::MelodyEditorPage::playAll_Click()
{
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		auto async = concurrency::create_task([this]
		{
			((App^)App::Current)->_player->stop();
			((App^)App::Current)->_player->playText(this->_texts, ref new SketchMusic::Cursor(_textRow->currentPosition));
		});
	}));
}

void StrokeEditor::MelodyEditorPage::_keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ args)
{
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
		case SketchMusic::View::KeyType::tempo:
			((App^)App::Current)->_player->_BPM = args->key->value;
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