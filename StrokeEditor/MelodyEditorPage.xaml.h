//
// MelodyEditorPage.xaml.h
// Объявление класса MelodyEditorPage
//

#pragma once

#include <map>
#include "MelodyEditorPage.g.h"
#include "CompositionEditorPage.xaml.h"

using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Navigation;

template<>
struct std::less<SketchMusic::View::Key^>
{
	bool operator() (SketchMusic::View::Key^ x, SketchMusic::View::Key^ y) const { return x->type < y->type; }
	typedef SketchMusic::View::Key^ first_argument_type;
	typedef SketchMusic::View::Key^ second_argument_type;
	typedef bool result_type;
};

namespace StrokeEditor
{
	/// <summary>
	/// Страница универсального редактора CompositionData
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class MelodyEditorPage sealed
	{
	public:
		MelodyEditorPage();

	protected:
		virtual void OnNavigatedTo(NavigationEventArgs^ e) override;
		void InitializePage();
		
	private:
		Windows::Foundation::EventRegistrationToken playerStateChangeToken;
		Windows::Foundation::EventRegistrationToken bpmChangeToken;
		Windows::Foundation::EventRegistrationToken curPosChangeToken;
		Windows::Foundation::EventRegistrationToken canUndoChangeToken;
		Windows::Foundation::EventRegistrationToken canRedoChangeToken;
		Windows::Foundation::EventRegistrationToken KeyDownToken;
		Windows::Foundation::EventRegistrationToken KeyUpToken;

		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		
		SketchMusic::CompositionData^ _texts;

		// команды
		SketchMusic::Commands::Command^ MoveSymCommand;
		SketchMusic::Commands::Command^ AddSymCommand;
		SketchMusic::Commands::Command^ DeleteSymCommand;
		SketchMusic::Commands::Command^ AddMultipleSymCommand;
		SketchMusic::Commands::Command^ DeleteMultipleSymCommand;
		SketchMusic::Commands::Command^ AddTextCommand;
		SketchMusic::Commands::Command^ DeleteTextCommand;

		// обработчики
		SketchMusic::Commands::Handler^ MoveSymHandler;
		SketchMusic::Commands::Handler^ ReverseMoveSymHandler;
		SketchMusic::Commands::Handler^ AddSymHandler;
		SketchMusic::Commands::Handler^ DeleteSymHandler;
		SketchMusic::Commands::Handler^ AddMultipleSymHandler;
		SketchMusic::Commands::Handler^ DeleteMultipleSymHandler;
		SketchMusic::Commands::Handler^ AddTextHandler;
		SketchMusic::Commands::Handler^ DeleteTextHandler;

		bool ctrlPressed;
		bool preventMenuClose;
		bool recording;
		bool cycling;
		bool appending;	// для задержки перед добавлением последующих нот с клавиатуры - чтобы и аккорды записывать, и курсор перемещать
		SketchMusic::View::ViewType viewType;

		static const int BASE_FREQ = 441;
		static const int BUFFER_LENGTH = (44100 / BASE_FREQ);
		static const int secMax = 1;

		std::multimap<SketchMusic::View::Key^, SketchMusic::Player::NoteOff^> notesPlayingMap;

		void SaveData();
		void playAll_Click();
		void _keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void _keyboard_KeyReleased(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		
		void UpdateChordViews(SketchMusic::Cursor^ pos);

		SketchMusic::Idea^ _idea;
		void LoadIdea();
		CompositionNavigationArgs^ _compositionArgs;
		Object^ _Sender;	// откуда мы пришли на эту страницу

		void LoadComposition();
		void OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args);
		void OnCursorPosChanged(Platform::Object ^sender, SketchMusic::Cursor^ pos);

		Windows::Foundation::Collections::IVector<SketchMusic::Instrument^>^ availableInstruments;
		void ListView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void DeleteTextBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

	private:

		void Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void MoveLeftCWBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void MoveRightCWBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PartsList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void MenuButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void UndoItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void RedoItem_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);


		void AddSymbolToText(SketchMusic::Text^ text, SketchMusic::PositionedSymbol^ symbol);
		void DeleteSymbolFromText(SketchMusic::Text^ text, SketchMusic::PositionedSymbol^ symbol);
		void Backspace();
		void OnCanRedoChanged(Platform::Object ^sender, bool args);
		void OnCanUndoChanged(Platform::Object ^sender, bool args);
		void OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
		void OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
		void OnSymbolPressed(Platform::Object ^sender, SketchMusic::PositionedSymbol ^args);
		void TextsList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void TextsFlyout_Opened(Platform::Object^ sender, Platform::Object^ e);
	};

	public ref class MelodyEditorArgs sealed
	{
	public:
		MelodyEditorArgs(Object^ sender, Object^ args) { Sender = sender; Args = args; }

		property Object^ Sender;
		property Object^ Args;
	};
}
