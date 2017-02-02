//
// AddIdeaDialog.xaml.cpp
// Реализация класса AddIdeaDialog
//

#include "pch.h"
#include "AddIdeaDialog.xaml.h"

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

using namespace SketchMusic;

// Документацию по шаблону элемента диалогового окна содержимого см. в разделе http://go.microsoft.com/fwlink/?LinkId=234238

StrokeEditor::AddIdeaDialog::AddIdeaDialog()
{
	InitializeComponent();
	
	auto vect = ref new Platform::Collections::Vector<IdeaCategoryEnum>;
	vect->Append((IdeaCategoryEnum)1);
	vect->Append((IdeaCategoryEnum)2);
	vect->Append((IdeaCategoryEnum)6);
	vect->Append((IdeaCategoryEnum)10);
	vect->Append((IdeaCategoryEnum)11);
	vect->Append((IdeaCategoryEnum)13);
	vect->Append((IdeaCategoryEnum)15);
	vect->Append((IdeaCategoryEnum)16);
	vect->Append((IdeaCategoryEnum)20);
	vect->Append((IdeaCategoryEnum)30);
	vect->Append((IdeaCategoryEnum)90);
	vect->Append((IdeaCategoryEnum)100);
	CategoryGrid->ItemsSource = vect;
}

void StrokeEditor::AddIdeaDialog::ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
	if (CategoryGrid->SelectedItem)
	{
		NewIdea = ref new Idea(nullptr, (IdeaCategoryEnum)CategoryGrid->SelectedItem);
	}
}

void StrokeEditor::AddIdeaDialog::ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
}


void StrokeEditor::AddIdeaDialog::CategoryGrid_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	
}
