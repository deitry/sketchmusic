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

	protected:
		void InitializePage();
		virtual void OnNavigatedTo(NavigationEventArgs^ e) override;
		virtual void OnNavigatedFrom(NavigationEventArgs^ e) override;

	public:
		//int PushIdea(SketchMusic::Idea^ idea);	// послать в бд
		//int UpdateIdea(SketchMusic::Idea^ idea);	// послать в бд
		//int InsertIdea(SketchMusic::Idea^ idea);	// послать в бд
		//int DeleteIdea(SketchMusic::Idea^ idea);	// послать в бд

	private:
		Windows::Foundation::EventRegistrationToken playerStateChangeToken;

		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void RefreshList();	// Обновить список в соответствии с выбранным фильтром

		static Windows::Foundation::Collections::IVector<SketchMusic::Idea^>^ ideaLibrary;
		void RefreshBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		//static Platform::String^ StringFromAscIIChars(std::string & chars);
		static int sqlite_readentry_callback(void *unused, int count, char **data, char **columns);
		void LibView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void GoMenuBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CreateEntryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void FilterList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		bool last3DaysFilter;
		bool rating5Higher;
		bool todoTagsFilter;
		bool baseTagsFilter;
		bool notBaseTagsFilter;
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args);
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class RatingToColorConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		// на основе клавиши выдаёт её графическое отображение
		// TODO : для всякого рода перемещений, стрелочек и так далее сделать красиво
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;

			int rating = ((Windows::Foundation::IPropertyValue^)value)->GetInt32();
			if (rating == 0) return nullptr;

			if (rating >= 10) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Blue);
			if (rating >= 8) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Green);
			if (rating >= 6) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::YellowGreen);
			if (rating >= 4) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Goldenrod);
			if (rating >= 2) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red);
			if (rating >= 1) return ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::DarkRed);

			return nullptr;
		}

		// обращаю внимание, что при данном подходе не получится сделать convert и convertBack и получить тот же результат
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		RatingToColorConverter() {}
	};

	public ref class ObjectToBoolConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			if (value) return true;
			else return false;
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		ObjectToBoolConverter() {}
	};

	public ref class ObjectToBoolStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Object^ parameter, Platform::String^ language)
		{
			if (value) return L"\uE73A";
			else return L"\uE739";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			return nullptr;
		}

		ObjectToBoolStrConverter() {}
	};
}
