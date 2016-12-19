//
// App.xaml.h
// Объявление класса приложения.
//

#pragma once

#include <winsqlite/winsqlite3.h>
//#include <sqlite3.h>

#include "App.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Обеспечивает зависящее от конкретного приложения поведение, дополняющее класс Application по умолчанию.
	/// </summary>
	ref class App sealed
	{
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;
		
	internal:
		App();

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);

		sqlite3* libraryDB;

	};

	ref class Command;
	ref class CommandManager;
}
