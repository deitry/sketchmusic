//
// CompositionOverviewPage.xaml.cpp
// Реализация класса CompositionOverviewPage
//

#include "pch.h"
#include "CompositionOverviewPage.xaml.h"
#include "CompositionEditorPage.xaml.h"
#include "MainMenuPage.xaml.h"

using namespace StrokeEditor;

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
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace concurrency;

using namespace SketchMusic;

// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

CompositionOverviewPage::CompositionOverviewPage()
{
	InitializeComponent();

	compositionList = ref new Platform::Collections::Vector<StorageFile^>;

	InitializePage();
}

void StrokeEditor::CompositionOverviewPage::InitializePage()
{
	//folderPicker->SuggestedStartLocation = PickerLocationId::MusicLibrary;
	
	//auto async = create_task(folderPicker->PickSingleFolderAsync()
	//).then([=](task<StorageFolder^> getFolderTask)
	//{
	//	auto folder = getFolderTask.get();
	//	if (folder != nullptr)
	//	{
	//		// Application now has read/write access to all contents in the picked folder (including other sub-folder contents)
	//		Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace("PickedFolderToken", folder);
	//	}
	//	return create_task(folder->GetFilesAsync());
	//}).then([=](task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^> resourceListTask)
	//{
	//	auto list = resourceListTask.get();
	//	for (auto&& file : list)
	//	{
	//		compositionList->Append(file->Name);
	//	}
	//});

	//async.wait();
	//auto folder = folderPicker->PickSingleFolderAsync();

	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	FolderPicker^ folderPicker = ref new FolderPicker;
	folderPicker->FileTypeFilter->Append("*");
	//return
	cancellation_token_source filePickerToken;
	auto openPickerTask = create_task(folderPicker->PickSingleFolderAsync(), filePickerToken.get_token());

	//StorageFolder^ folder;
	//this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]()
	//{
	auto myTask = create_task([=]
	{
		task<StorageFolder^> getFolderAsync;
		if (localSettings->Values->HasKey("workspace_path"))
		{
			auto path = (String^)localSettings->Values->Lookup("workspace_path");
			return create_task(StorageFolder::GetFolderFromPathAsync(path));
		}
		else
		return create_task([]() -> StorageFolder^ {return nullptr;});
	}).then([=](task<StorageFolder^> getFolder) {
		StorageFolder^ folder = nullptr;
		try
		{
			folder = getFolder.get();
			filePickerToken.cancel();
			return folder;
		}
		catch (AccessDeniedException^ e) {}
		catch (concurrency::invalid_operation* e) {}
		catch (Exception^ e) {}
		catch (std::exception* e) {}
		return openPickerTask.get();
	})
	.then([=](StorageFolder^ folder)
	{
		//auto folder = folder1 ? folder1 : getFolder.get();
		if (folder != nullptr)
		{
			auto fullPath = folder->Path;
			localSettings->Values->Insert("workspace_path", fullPath);
			Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace("PickedFolderToken", folder);

			Workspace = folder;

			create_task(folder->GetFilesAsync()).then([=](Windows::Foundation::Collections::IVectorView<StorageFile^>^ fileList)
			{
				compositionList->Clear();
				for (auto&& file : fileList)
				{
					compositionList->Append(file);
				}
				this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
				{
					CompositionListView->ItemsSource = compositionList;
				}));
			});
		}
	});
}


void StrokeEditor::CompositionOverviewPage::GoMenuBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid));
}


void StrokeEditor::CompositionOverviewPage::FilterList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{

}


void StrokeEditor::CompositionOverviewPage::RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionOverviewPage::CreateEntryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::CompositionEditorPage::typeid), ref new CompositionNavigationArgs(Workspace, nullptr, nullptr));
}


void StrokeEditor::CompositionOverviewPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Frame->Navigate(TypeName(StrokeEditor::CompositionEditorPage::typeid), ref new CompositionNavigationArgs(Workspace, (StorageFile^) CompositionListView->SelectedItem, nullptr));
}


void StrokeEditor::CompositionOverviewPage::SetWorkingFolderBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
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
				localSettings->Values->Insert("workspace_path", folder->Path);
				
				Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace("PickedFolderToken", folder);

				Workspace = folder;

				create_task(folder->GetFilesAsync()).then([=](Windows::Foundation::Collections::IVectorView<StorageFile^>^ fileList)
				{
					for (auto file : fileList)
					{
						compositionList->Append(file);
					}
					this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
					{
						auto listView = dynamic_cast<ListView^>(CompositionListView);
						if (listView)
						{
							listView->ItemsSource = compositionList;
						}
					}));
				});
			}
		});
	}));
}


void StrokeEditor::CompositionOverviewPage::PlayBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void StrokeEditor::CompositionOverviewPage::CompositionListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	EditBtn->IsEnabled = CompositionListView->SelectedItem == nullptr ? false : true;
}
