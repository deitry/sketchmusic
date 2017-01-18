//
// CompositionEditorPage.xaml.cpp
// Реализация класса CompositionEditorPage
//

#include <string>
#include "pch.h"
#include "CompositionEditorPage.xaml.h"
#include "CompositionOverviewPage.xaml.h"
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
using namespace Windows::Data::Json;
using namespace SketchMusic;
using namespace concurrency;


// Шаблон элемента пустой страницы задокументирован по адресу http://go.microsoft.com/fwlink/?LinkId=234238

CompositionEditorPage::CompositionEditorPage()
{
	InitializeComponent();

	InitializePage();
}

void StrokeEditor::CompositionEditorPage::InitializePage()
{
	AreButtonsEnabled(false);
}

void StrokeEditor::CompositionEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	auto args = reinterpret_cast<CompositionNavigationArgs^>(e->Parameter);
	if (args && args->File != nullptr)
	{
		CompositionFile = args->File;
		create_task(FileIO::ReadTextAsync(CompositionFile))
			.then([=](String^ text)
		{
			if (text)
			{
				JsonObject^ json = Windows::Data::Json::JsonObject::Parse(text);
				CompositionProject = Composition::Deserialize(json);
			}
			else
				CompositionProject = ref new Composition;

			SetParts(CompositionProject->Data->getParts());
			AreButtonsEnabled(true);
		});
	}
	else
	{
		// создать новый в указанной папке
		auto dialog = ref new ContentDialog;
		dialog->Title = "Новый проект";
		dialog->Content = "Введите имя файла для нового проекта";
		dialog->PrimaryButtonText = "Создать";
		dialog->SecondaryButtonText = "Отмена";
		
		Grid^ grid = ref new Grid;
		TextBox^ fileNameTxt = ref new TextBox;
		grid->Children->Append(fileNameTxt);
		dialog->Content = grid;
		dialog->PrimaryButtonText = "Ok";
		dialog->SecondaryButtonText = "Cancel";
		
		this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low, ref new Windows::UI::Core::DispatchedHandler([=]()
		{
			cancellation_token_source cancelTokenSource;
			create_task(dialog->ShowAsync() , cancelTokenSource.get_token())
				.then([=] (ContentDialogResult result) {
				if (result == ContentDialogResult::Primary)
				{
					String^ name = fileNameTxt->Text;
					name += ".jsm";
					return args->Workspace->CreateFileAsync(name);
				}
				else
				{
					//cancelTokenSource.cancel();
					cancel_current_task();
				}
			}).then([=](StorageFile^ file)
			{
				if (file)
				{
					CompositionFile = file;
				}
			}, cancelTokenSource.get_token()).then([=]
			{
				return FileIO::ReadTextAsync(CompositionFile);
			}, cancelTokenSource.get_token()).then([=](String^ text)
			{
				if (text)
				{
					JsonObject^ json = Windows::Data::Json::JsonObject::Parse(text);
					CompositionProject = Composition::Deserialize(json);
				}
				else
					CompositionProject = ref new Composition;

				SetParts(CompositionProject->Data->getParts());
				AreButtonsEnabled(true);
			}, cancelTokenSource.get_token());
		}));
	}
}



void StrokeEditor::CompositionEditorPage::SaveComposition()
{
	// применяем все изменения
	CompositionProject->Data->ApplyParts(CompositionView->Parts);
	// сериализуем композицию
	auto json = CompositionProject->Serialize();
	// пишем в файл
	create_task(FileIO::WriteTextAsync(CompositionFile, json->Stringify()))
		.then([=] {
		auto dialog = ref new ContentDialog;
		dialog->Title = "Сохранение в файл";
		dialog->Content = "Успешно сохранено";
		dialog->PrimaryButtonText = "Ок";
		return dialog->ShowAsync();
	});
}

void StrokeEditor::CompositionEditorPage::AreButtonsEnabled(bool isEnabled)
{
	SaveBtn->IsEnabled = isEnabled;
	EditBtn->IsEnabled = isEnabled;
	AddPartBtn->IsEnabled = isEnabled;
	DeleteBtn->IsEnabled = isEnabled;
	OkBtn->IsEnabled = isEnabled;
	PlayCompositionBtn->IsEnabled = isEnabled;
}

void StrokeEditor::CompositionEditorPage::SetParts(IObservableVector<PartDefinition^>^ parts)
{
	CompositionView->SetParts(parts);
	CompositionPartList->ItemsSource = parts;
	parts->VectorChanged += 
		ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PartDefinition ^>(this, &StrokeEditor::CompositionEditorPage::OnVectorChanged);
}


void StrokeEditor::CompositionEditorPage::HomeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// идём в главное меню 
	this->Frame->Navigate(TypeName(StrokeEditor::MainMenuPage::typeid), nullptr);
}


void StrokeEditor::CompositionEditorPage::DeleteBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// удаляем данный проект
}


void StrokeEditor::CompositionEditorPage::CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// возвращаемся к обзору без сохранения изменений
}


void StrokeEditor::CompositionEditorPage::SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// сохраняем изменения в файле
	SaveComposition();
}


void StrokeEditor::CompositionEditorPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// редактируем композицию, переставляя курсор на начало выбранной части
}


void StrokeEditor::CompositionEditorPage::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// сохраняем изменения в файле
	SaveComposition();
	// переходим к обзору проектов
	this->Frame->Navigate(TypeName(StrokeEditor::CompositionOverviewPage::typeid), nullptr);
}


void StrokeEditor::CompositionEditorPage::Page_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	auto width = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->VisibleBounds.Width - CompositionView->Margin.Left - CompositionView->Margin.Right;
	CompositionView->Width = width;
	CompositionView->UpdateSize();

	CompositionPartList->Width = width;
}

void StrokeEditor::CompositionEditorPage::CompositionPartList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{

}


void StrokeEditor::CompositionEditorPage::CompositionPartList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	DeletePartBtn->IsEnabled = CompositionPartList->SelectedItem ? true : false;
	EditPartBtn->IsEnabled = CompositionPartList->SelectedItem ? true : false;
	CompositionView->SelectedItem = (PartDefinition^)CompositionPartList->SelectedItem;
}


void StrokeEditor::CompositionEditorPage::DeletePartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	unsigned int ind;
	auto selected = CompositionPartList->SelectedItem;
	if (CompositionView->Parts->IndexOf((PartDefinition^)selected, &ind))
	{
		CompositionView->SelectedItem = nullptr;
		CompositionView->Parts->RemoveAt(ind);
	}
}


void StrokeEditor::CompositionEditorPage::AcceptAddPartButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	AddPartFlyout->Hide();
	
	PartDefinition^ part = ref new PartDefinition;
	part->Length = std::stoi(PartLength->Text->Data());
	part->original = ref new SNewPart;
	part->original->category = PartCatTxt->Text;
	part->original->Name = PartNameTxt->Text;
	part->original->dynamic = (DynamicCategory)PartCatComboBox->SelectedIndex;

	CompositionView->Parts->Append(part);
}


void StrokeEditor::CompositionEditorPage::EditPartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	AcceptAddPartBtn->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	AcceptEditPartBtn->Visibility = Windows::UI::Xaml::Visibility::Visible;

	PartDefinition^ part = (PartDefinition^)CompositionPartList->SelectedItem;
	PartCatTxt->Text = part->original->category;
	PartNameTxt->Text = part->original->Name;
	PartCatComboBox->SelectedIndex = (int)part->original->dynamic;
	PartLength->Text = part->Length.ToString();

	AddPartFlyout->ShowAt(EditPartBtn);
}


void StrokeEditor::CompositionEditorPage::PartLength_TextChanging(Windows::UI::Xaml::Controls::TextBox^ sender, Windows::UI::Xaml::Controls::TextBoxTextChangingEventArgs^ args)
{
	if (sender->SelectionStart != 0)
	{
		std::wstring lastChar = sender->Text->Data();
		int pos = sender->SelectionStart - 1;
		lastChar = lastChar.substr(pos,1);
		if (!iswdigit(lastChar.at(0)))
		{
			std::wstring wstr = sender->Text->Data();
			wstr = wstr.substr(0,pos);
			sender->Text = ref new Platform::String(wstr.data());
			sender->SelectionStart = pos;
		}
	}
}

void StrokeEditor::CompositionEditorPage::OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs ^ args)
{
	switch (args->CollectionChange)
	{
	case CollectionChange::ItemInserted:
		CompositionPartList->ItemsSource = nullptr;
		CompositionPartList->ItemsSource = CompositionView->Parts;
		break;
	}
}


void StrokeEditor::CompositionEditorPage::AddPartBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	AcceptAddPartBtn->Visibility = Windows::UI::Xaml::Visibility::Visible;
	AcceptEditPartBtn->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}


void StrokeEditor::CompositionEditorPage::AcceptEditPartButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	AddPartFlyout->Hide();

	PartDefinition^ part = (PartDefinition^)CompositionPartList->SelectedItem;
	part->Length = std::stoi(PartLength->Text->Data());
	part->original->category = PartCatTxt->Text;
	part->original->Name = PartNameTxt->Text;
	part->original->dynamic = (DynamicCategory)PartCatComboBox->SelectedIndex;

	CompositionPartList->ItemsSource = nullptr;
	CompositionPartList->ItemsSource = CompositionView->Parts;
}
