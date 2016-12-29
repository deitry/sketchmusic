//
// App.xaml.h
// Объявление класса приложения.
//

#pragma once

#include <sqlite3.h>

#include "App.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Обеспечивает зависящее от конкретного приложения поведение, дополняющее класс Application по умолчанию.
	/// </summary>
	ref class App sealed
	{
	public:
		void InitializeApp();
		//virtual void OnResuming(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e) override;
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;
		
	internal:
		App();

		sqlite3* libraryDB;
		SketchMusic::Player::Player^ _player;
		SketchMusic::Commands::CommandManager^ _manager;


		void OpenLibrary(Platform::String^ dbName);
		int InsertIdea(SketchMusic::Idea^ idea);
		int UpdateIdea(SketchMusic::Idea^ idea);
		int DeleteIdea(SketchMusic::Idea^ idea);
		void ShowNotification(Platform::String^ message);

		void Play(Windows::Foundation::Collections::IVector<SketchMusic::Text^>^ texts, SketchMusic::Cursor^ cursor);

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	};

	ref class Command;
	ref class CommandManager;
}
