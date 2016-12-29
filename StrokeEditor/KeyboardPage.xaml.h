//
// MainPage.xaml.h
// Объявление класса MainPage.
//

#pragma once

#include "KeyboardPage.g.h"

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
	public ref class KeyboardPage sealed
	{
	private:
		//SketchMusic::Player::Player^ _player;
		Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ _texts;

		//SketchMusic::Commands::CommandManager^ _manager;
		
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

	public:
		KeyboardPage();

	private:
		void playAll_Click();
		void _keyboard_KeyboardPressed(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void _keyboard_KeyReleased(Platform::Object^ sender, SketchMusic::View::KeyboardEventArgs^ e);
		void ListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	public ref class InstrumentToTextConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			SketchMusic::Instrument^ instr = dynamic_cast<SketchMusic::Instrument^>(value);
			if (instr == nullptr) return "";

			return instr->_name;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		InstrumentToTextConverter() {}
	};

}

