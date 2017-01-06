//
// LibraryEntryPage.xaml.h
// Объявление класса LibraryEntryPage
//

#pragma once

#include "LibraryEntryPage.g.h"
#include "LibraryOverviewPage.xaml.h"

using namespace Windows::UI::Xaml::Navigation;

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class LibraryEntryPage sealed
	{
	public:
		LibraryEntryPage();
		property Platform::Boolean _isRead;

	protected:
		void InitializePage();
		virtual void OnNavigatedTo(NavigationEventArgs^ e) override;

	private:
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void NotifyBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void SaveEntry();
		
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ExtendBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		//StrokeEditor::LibraryOverviewPage^ _library;
		void HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	public ref class LibraryEntryNavigationArgs sealed
	{
	public:
		LibraryEntryNavigationArgs() {}
		LibraryEntryNavigationArgs(Platform::Object^ _idea, Platform::Boolean _isRead) //, StrokeEditor::LibraryOverviewPage^ _library 
		{
			idea = _idea; isRead = _isRead; //library = _library;
		}

		property Platform::Object^ idea;
		property Platform::Boolean isRead;
		//property StrokeEditor::LibraryOverviewPage^ library;
	};
}
