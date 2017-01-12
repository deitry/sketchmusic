//
// CompositionEditorPage.xaml.h
// Объявление класса CompositionEditorPage
//

#pragma once

#include "CompositionEditorPage.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class CompositionEditorPage sealed
	{
	public:
		CompositionEditorPage();
	private:
		void HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void CompositionPartList_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
		void CompositionPartList_DragItemsStarting(Platform::Object^ sender, Windows::UI::Xaml::Controls::DragItemsStartingEventArgs^ e);
		void CompositionPartList_DropCompleted(Windows::UI::Xaml::UIElement^ sender, Windows::UI::Xaml::DropCompletedEventArgs^ args);
		void Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
	};
}
