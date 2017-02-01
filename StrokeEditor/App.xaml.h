//
// App.xaml.h
// Объявление класса приложения.
//

#pragma once

#include <sqlite3.h>

#include "App.g.h"

using namespace SketchMusic;
using namespace Windows::Storage;

namespace StrokeEditor
{
	public ref class CompositionNavigationArgs sealed
	{
	public:
		CompositionNavigationArgs() {}
		CompositionNavigationArgs(StorageFolder^ _folder, StorageFile^ _file, Composition^ _project)
		{
			Workspace = _folder; File = _file; Project = _project; Selected = -1;
		}
		CompositionNavigationArgs(StorageFolder^ _folder, StorageFile^ _file, Composition^ _project, int selected)
		{
			Workspace = _folder; File = _file; Project = _project; Selected = selected;
		}

		property StorageFile^ File;
		property StorageFolder^ Workspace;
		property Composition^ Project;
		property int Selected;
	};

	//ref class Command;
	//ref class CommandManager;

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

		// для сохранения в OnSuspending
		SketchMusic::Idea^ _CurrentIdea;
		StrokeEditor::CompositionNavigationArgs^ _CurrentCompositionArgs;
		Windows::Foundation::Collections::IObservableVector<PartDefinition^>^ _CurrentParts;

		sqlite3* libraryDB;
		SketchMusic::Player::Player^ _player;
		SketchMusic::Commands::CommandManager^ _manager;
		Windows::Foundation::Collections::IVector<SketchMusic::Idea^>^ ideaLibrary;
		Windows::Storage::StorageFile^ DebugFile;
		void WriteToDebugFile(Platform::String^ str);

		void OpenLibrary(Platform::String^ dbPath);
		void SaveLibrary(Platform::String^ dbPath);
		void CloseLibrary();

		void OpenLocalOrExternalLibrary(Platform::String^ dbName);

		int InsertIdea(SketchMusic::Idea^ idea);
		int UpdateIdea(SketchMusic::Idea^ idea);
		int DeleteIdea(SketchMusic::Idea^ idea);
		void ShowNotification(Platform::String^ message);
		void SaveData();
		void SetTile(Platform::String^ str);

		void Play(SketchMusic::CompositionData^ texts, SketchMusic::Cursor^ cursor);

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	};
}
