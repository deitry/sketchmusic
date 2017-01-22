//
// CompositionEditorPage.xaml.cpp
// Реализация класса CompositionEditorPage
//

#include <string>
#include "pch.h"
#include "CompositionEditorPage.xaml.h"
#include "CompositionOverviewPage.xaml.h"
#include "MelodyEditorPage.xaml.h"
#include "MainMenuPage.xaml.h"
#include "EditCompositionDialog.xaml.h"

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
	Windows::Storage::ApplicationDataContainer^ localSettings =
		Windows::Storage::ApplicationData::Current->LocalSettings;

	if (localSettings)
	{
		if (localSettings->Values->HasKey("need_metronome"))
		{
			((App^)App::Current)->_player->needMetronome = (bool)localSettings->Values->Lookup("need_metronome");
			NeedMetronomeChbx->IsChecked = ((App^)App::Current)->_player->needMetronome;
		}
		else {
			((App^)App::Current)->_player->needMetronome = true;
			NeedMetronomeChbx->IsChecked = true;
		}

		if (localSettings->Values->HasKey("need_play_generic"))
		{
			((App^)App::Current)->_player->needPlayGeneric = (bool)localSettings->Values->Lookup("need_play_generic");
			NeedGenericChbx->IsChecked = ((App^)App::Current)->_player->needPlayGeneric;
		}
		else {
			((App^)App::Current)->_player->needPlayGeneric = true;
			NeedGenericChbx->IsChecked = true;
		}
	}

	curPosChangeToken = 
		((App^)App::Current)->_player->CursorPosChanged 
		+= ref new Windows::Foundation::EventHandler<SketchMusic::Cursor ^>(this, &StrokeEditor::CompositionEditorPage::OnCursorPosChanged);
	playerStateChangeToken =
		((App^)App::Current)->_player->StateChanged 
		+= ref new Windows::Foundation::EventHandler<SketchMusic::Player::PlayerState>(this, &StrokeEditor::CompositionEditorPage::OnStateChanged);
}

void StrokeEditor::CompositionEditorPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	auto args = reinterpret_cast<CompositionNavigationArgs^>(e->Parameter);
	if (args == nullptr) return;
	
	// если передаём целиком композицию, возвращаясь, например, из MelodyEditor
	if (args->Project != nullptr)
	{
		CompositionFile = args->File;
		CompositionProject = args->Project;

		SetParts(CompositionProject->Data->getParts());
		AreButtonsEnabled(true);
	}
	else if (args->File != nullptr)
	{
		CompositionFile = args->File;
		//CompositionWorkspace = args->Workspace;
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

			if (CompositionProject->Header->Name == nullptr || CompositionProject->Header->Name == "")
				CompositionProject->Header->Name = CompositionFile->Name;

			CompositionProject->Header->FileName = CompositionFile->Name;
			SetParts(CompositionProject->Data->getParts());
			AreButtonsEnabled(true);

			// "сохраняем" нашу композицию в параметрах приложения для сохранения на случай внезапного выключения
			args->File = CompositionFile;
			// Workspace, полагаю, никуда не делся из аргументов, если он там был
			args->Project = CompositionProject;
			((App^)App::Current)->_CurrentCompositionArgs = args;
		});
	}
	else if (args->Workspace != nullptr)
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

				CompositionProject->Header->Name = fileNameTxt->Text;
				CompositionProject->Header->FileName = fileNameTxt->Text + ".jsm";
				SetParts(CompositionProject->Data->getParts());
				AreButtonsEnabled(true);

				// "сохраняем" нашу композицию в параметрах приложения для сохранения на случай внезапного выключения
				args->File = CompositionFile;
				// Workspace, полагаю, никуда не делся из аргументов, если он там был
				args->Project = CompositionProject;
				((App^)App::Current)->_CurrentCompositionArgs = args;
			}, cancelTokenSource.get_token());
		}));
	}
}

void StrokeEditor::CompositionEditorPage::OnNavigatedFrom(NavigationEventArgs ^ e)
{
	// обнуляем "текущую композицию"
	((App^)App::Current)->_CurrentCompositionArgs = nullptr;
	((App^)App::Current)->_CurrentParts = nullptr;

	// отписка от события
	(((App^)App::Current)->_player)->StateChanged -= playerStateChangeToken;
	(((App^)App::Current)->_player)->CursorPosChanged -= curPosChangeToken;
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
	AddPartBtn->IsEnabled = isEnabled;
	DeleteBtn->IsEnabled = isEnabled;
	OkBtn->IsEnabled = isEnabled;
	PlayCompositionBtn->IsEnabled = isEnabled;
}

void StrokeEditor::CompositionEditorPage::SetParts(IObservableVector<PartDefinition^>^ parts)
{
	CompositionView->SetParts(parts);
	((App^)App::Current)->_CurrentParts = parts;
	CompositionPartList->ItemsSource = parts;
	parts->VectorChanged += 
		ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PartDefinition ^>(this, &StrokeEditor::CompositionEditorPage::OnVectorChanged);
	TitleTxt->Text = CompositionProject->Header->Name;
	UpdateTotalLength();
}

void StrokeEditor::CompositionEditorPage::UpdateTotalLength()
{
	int totalLength = 0;
	for (auto&& part : CompositionView->Parts)
	{
		totalLength += part->Length;
	}

	CompositionSlider->Maximum = totalLength;

	totalLength = totalLength * 60 / CompositionProject->Data->BPM;	// /BPM
	int minutes = totalLength / 60;
	int seconds = totalLength % 60;
	FullLengthTxt->Text = minutes.ToString() + ":" + ((seconds < 10) ? "0" : "") + seconds.ToString();
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
	this->Frame->Navigate(TypeName(StrokeEditor::CompositionOverviewPage::typeid), nullptr);
}


void StrokeEditor::CompositionEditorPage::SaveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// сохраняем изменения в файле
	SaveComposition();
}


void StrokeEditor::CompositionEditorPage::EditBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// применяем все изменения
	CompositionProject->Data->ApplyParts(CompositionView->Parts);
	// редактируем композицию, переставляя курсор на начало выбранной части
	this->Frame->Navigate(TypeName(StrokeEditor::MelodyEditorPage::typeid), ref new CompositionNavigationArgs(nullptr, CompositionFile, CompositionProject, CompositionPartList->SelectedIndex));
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
	bool hasSelection = CompositionPartList->SelectedItem ? true : false;
	DeletePartBtn->IsEnabled = hasSelection;
	EditPartBtn->IsEnabled = hasSelection;
	EditBtn->IsEnabled = hasSelection;
	CompositionView->SelectedItem = (PartDefinition^)CompositionPartList->SelectedItem;
	if (hasSelection)
	{
		int cur = 0;
		for (auto&& p : CompositionView->Parts)
		{
			if (static_cast<Object^>(p) == CompositionView->SelectedItem) break;
			cur += p->Length;
		}
		CompositionSlider->Value = cur;
	}
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
		
		UpdateTotalLength();
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


void StrokeEditor::CompositionEditorPage::TitleTxt_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	auto dialog = ref new EditCompositionDialog;
	dialog->DataContext = CompositionProject;
	dialog->PrimaryButtonText = "Ок";
	dialog->SecondaryButtonText = "Отмена";
	create_task(dialog->ShowAsync())
		.then([=](ContentDialogResult result)
	{
		if (result == ContentDialogResult::Primary)
		{
			this->Dispatcher->RunAsync(
				Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([=]() {
				SetParts(CompositionProject->Data->getParts());
			}));
		}
	});
}


void StrokeEditor::CompositionEditorPage::PlayCompositionBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (((App^)App::Current)->_player->_state != SketchMusic::Player::PlayerState::STOP)
	{
		((App^)App::Current)->_player->stop();
		return;
	}

	// приводим текст в соответствие со списком частей
	CompositionProject->Data->ApplyParts(CompositionView->Parts);

	// начинаем проигрывать с заданного места и пока не остановимся
	((App^)App::Current)->_player->StopAtLast = true;	// последний символ - последняя часть
	((App^)App::Current)->_player->needMetronome = NeedMetronomeChbx->IsChecked->Value;	// последний символ - последняя часть
	((App^)App::Current)->_player->needPlayGeneric= NeedGenericChbx->IsChecked->Value;		// последний символ - последняя часть
	((App^)App::Current)->_player->quantize = 1;		// для обновления положения слайдера нам больше не надо
	((App^)App::Current)->_player->precount = 0;
	int start = CompositionSlider->Value;
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]()
	{
		auto async = concurrency::create_task([=]
		{
			//((App^)App::Current)->_player->stop();
			((App^)App::Current)->_player->playText(CompositionProject->Data, ref new SketchMusic::Cursor(start));
		});
	}));
}


void StrokeEditor::CompositionEditorPage::NeedMetronomeChbx_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	((App^)App::Current)->_player->needMetronome = NeedMetronomeChbx->IsChecked->Value;
	Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert("need_metronome", ((App^)App::Current)->_player->needMetronome);
}


void StrokeEditor::CompositionEditorPage::NeedGenericChbx_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	((App^)App::Current)->_player->needPlayGeneric = NeedGenericChbx->IsChecked->Value;
	Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert("need_play_generic", ((App^)App::Current)->_player->needMetronome);
}


void StrokeEditor::CompositionEditorPage::OnStateChanged(Platform::Object ^sender, SketchMusic::Player::PlayerState args)
{
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
		switch (args)
		{
		case SketchMusic::Player::PlayerState::PLAY:
			PlayCompositionBtn->Content = L"\uE769";
			break;
		case SketchMusic::Player::PlayerState::STOP:
		case SketchMusic::Player::PlayerState::WAIT:
			PlayCompositionBtn->Content = L"\uE768";
			break;
		}
	}));
}


void StrokeEditor::CompositionEditorPage::OnCursorPosChanged(Platform::Object ^sender, SketchMusic::Cursor ^args)
{
	this->Dispatcher->RunAsync(
		Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
		CompositionSlider->Value = args->Beat;
	}));
}
