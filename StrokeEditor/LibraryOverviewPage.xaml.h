//
// LibraryOverviewPage.xaml.h
// Объявление класса LibraryOverviewPage
//

#pragma once

#include "LibraryOverviewPage.g.h"
#include "App.xaml.h"

using namespace Windows::UI::Xaml::Navigation;

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class LibraryOverviewPage sealed
	{
	public:
		LibraryOverviewPage();

	private:
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void RefreshList();	// Обновить список в соответствии с выбранным фильтром

		static Windows::Foundation::Collections::IVector<SketchMusic::Idea^>^ ideaLibrary;
		void RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		//static Platform::String^ StringFromAscIIChars(std::string & chars);
		static int sqlite_readentry_callback(void *unused, int count, char **data, char **columns);
	};
}
