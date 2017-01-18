//
// CompositionEditorPage.xaml.h
// Объявление класса CompositionEditorPage
//

#pragma once

#include "CompositionEditorPage.g.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace SketchMusic;

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
		property StorageFile^ CompositionFile;
		property Composition^ CompositionProject;

	protected:
		void InitializePage();
		virtual void OnNavigatedTo(NavigationEventArgs^ e) override;
		void SaveComposition();
		void AreButtonsEnabled(bool isEnabled);
		void SetParts(IObservableVector<PartDefinition^>^ parts);

	private:
		void HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void CompositionPartList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void CompositionPartList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void DeletePartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AcceptAddPartButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void EditPartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PartLength_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args);
		void OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs^ args);
		void AddPartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AcceptEditPartButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	public ref class CompositionNavigationArgs sealed
	{
	public:
		CompositionNavigationArgs() {}
		CompositionNavigationArgs(StorageFolder^ _folder, StorageFile^ _file)
		{
			Workspace = _folder; File = _file;
		}

		property StorageFile^ File;
		property StorageFolder^ Workspace;
		//property StrokeEditor::LibraryOverviewPage^ library;
	};
}
