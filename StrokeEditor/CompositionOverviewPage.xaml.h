//
// CompositionOverviewPage.xaml.h
// Объявление класса CompositionOverviewPage
//

#pragma once

#include "CompositionOverviewPage.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class CompositionOverviewPage sealed
	{
	public:
		CompositionOverviewPage();
		property Windows::Storage::StorageFolder^ Workspace
		{
			Windows::Storage::StorageFolder^ get() { return m_workspace; }
			void set(Windows::Storage::StorageFolder^ folder) 
			{ 
				m_workspace = folder; this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
				{
					TitleTxt->Text = folder->Name;
				})); 
			}
		}
	private:
		Windows::Storage::StorageFolder^ m_workspace;

		void GoMenuBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void FilterList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CreateEntryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void InitializePage();
		
		Windows::Foundation::Collections::IVector < Windows::Storage::StorageFile^ > ^ compositionList;
		void SetWorkspace();

		void SetWorkingFolderBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CompositionListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void Grid_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e);
		void Grid_RightTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs^ e);

		void OpenContextMenu(Platform::Object^ ctrl, Windows::Foundation::Point point);
		FrameworkElement^ _CurrentContext;
		void OverviewItemContextMenu_Opened(Platform::Object^ sender, Platform::Object^ e);
		void OverviewItemContextMenu_Closed(Platform::Object^ sender, Platform::Object^ e);
		void TitleTxt_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class FileToTextConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			auto file = dynamic_cast<Windows::Storage::StorageFile^>(value);
			
			return file->Name;
		}

		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
			//return ref new SketchMusic::PositionedSymbol(ref new SketchMusic::Cursor, ref new SketchMusic::SNote);
		}

		FileToTextConverter() {}
	};
}
