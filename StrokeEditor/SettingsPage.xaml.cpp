//
// SettingsPage.xaml.cpp
// Реализация класса SettingsPage
//

#include "pch.h"
#include "SettingsPage.xaml.h"
#include "ManageInstrumentsDialog.xaml.h"
#include "MainMenuPage.xaml.h"

using namespace StrokeEditor;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Interop;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

SettingsPage::SettingsPage()
{
	InitializeComponent();
}


void StrokeEditor::SettingsPage::SetDefaultInstrBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть диалог с выбором инструмента
	auto dialog = ref new ManageInstrumentsDialog;
	
	create_task(dialog->ShowAsync())
		.then([=](ContentDialogResult result)
	{
		if (result == ContentDialogResult::Primary)
		{
			this->Dispatcher->RunAsync(
				Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([=]() {
				
				Windows::Storage::ApplicationDataContainer^ localSettings =
					Windows::Storage::ApplicationData::Current->LocalSettings;

				localSettings->Values->Insert("default_instr", dialog->Selected->Serialize()->Stringify());

			}));
		}
	});
}


void StrokeEditor::SettingsPage::SetSF2FolderBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// открыть диалог с выбором папки
	FolderPicker^ folderPicker = ref new FolderPicker;
	folderPicker->FileTypeFilter->Append("*");
	
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
	{
		create_task(folderPicker->PickSingleFolderAsync())
			.then([=](StorageFolder^ folder)
		{
			if (folder != nullptr)
			{
				Windows::Storage::ApplicationDataContainer^ localSettings =
					Windows::Storage::ApplicationData::Current->LocalSettings;
				localSettings->Values->Insert("sf2_path", folder->Path);

				Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace("SF2Path", folder);
			}
		});
	}));
}


void StrokeEditor::SettingsPage::GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid), nullptr);
}
