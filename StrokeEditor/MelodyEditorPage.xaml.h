//
// MelodyEditorPage.xaml.h
// Объявление класса MelodyEditorPage
//

#pragma once

#include "MelodyEditorPage.g.h"

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
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
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

		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		//void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		SketchMusic::CompositionData^ _texts;

		SketchMusic::Commands::Handler^ moveSym;
		SketchMusic::Commands::Handler^ addSym;
		SketchMusic::Commands::Handler^ deleteSym;

		bool recording;
		bool cycling;
		bool appending;	// для задержки перед добавлением последующих нот с клавиатуры - чтобы и аккорды записывать, и курсор перемещать

		static const int BASE_FREQ = 441;
		static const int BUFFER_LENGTH = (44100 / BASE_FREQ);
		static const int secMax = 1;

		std::multimap<SketchMusic::View::Key^, SketchMusic::Player::NoteOff^> notesPlayingMap;

		void playAll_Click();
		void _keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void _keyboard_KeyReleased(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		//void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		SketchMusic::Idea^ _idea;
		void OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args);
		void OnBpmChanged(Platform::Object ^sender, float args);
		void OnCursorPosChanged(Platform::Object ^sender, SketchMusic::Cursor^ pos);

		Windows::Foundation::Collections::IVector<SketchMusic::Instrument^>^ availableInstruments;
		void ListView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void DeleteTextBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

	private:

		void HamburgerButton_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
		{
			mySplitView->IsPaneOpen = !mySplitView->IsPaneOpen;
		}
		void menu_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
	};
}
