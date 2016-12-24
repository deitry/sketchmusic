//
// MelodyEditorPage.xaml.h
// Объявление класса MelodyEditorPage
//

#pragma once

#include "MelodyEditorPage.g.h"
#include "KeyboardPage.xaml.h"

using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Navigation;

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

	private:
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		//void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		SketchMusic::Player::Player^ _player;
		Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ _texts;

		SketchMusic::Commands::CommandManager^ _manager;

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
		//void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		//void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		SketchMusic::Idea^ _idea;
	};
}
