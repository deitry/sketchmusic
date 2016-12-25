﻿//
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
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;
		
	internal:
		App();

		sqlite3* libraryDB;

		int InsertIdea(SketchMusic::Idea^ idea);
		int UpdateIdea(SketchMusic::Idea^ idea);
		int DeleteIdea(SketchMusic::Idea^ idea);

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	};

	ref class Command;
	ref class CommandManager;
}
