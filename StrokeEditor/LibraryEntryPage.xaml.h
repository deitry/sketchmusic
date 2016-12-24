//
// LibraryEntryPage.xaml.h
// Объявление класса LibraryEntryPage
//

#pragma once

#include "LibraryEntryPage.g.h"

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
		virtual void OnNavigatedTo(NavigationEventArgs^ e) override;

	private:
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void NotifyBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	public ref class LibraryEntryNavigationArgs sealed
	{
	public:
		LibraryEntryNavigationArgs() {}
		LibraryEntryNavigationArgs(Platform::Object^ _idea, Platform::Boolean _isRead) { idea = _idea; isRead = _isRead; }

		property Platform::Object^ idea;
		property Platform::Boolean isRead;
	};
}
