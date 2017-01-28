//
// ManageInstrumentsDialog.xaml.cpp
// Реализация класса ManageInstrumentsDialog
//

#include "pch.h"
#include "ManageInstrumentsDialog.xaml.h"
#include "App.xaml.h"

using namespace StrokeEditor;

using namespace concurrency;
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

StrokeEditor::ManageInstrumentsDialog::ManageInstrumentsDialog()
{
	InitializeComponent();

	//create_task([=] {
	//	test = ref new CompositionData;
	//	testText = ref new SketchMusic::Text;
	//	testText->addSymbol(ref new Cursor(0), ref new SNote(3));
	//	testText->addSymbol(ref new Cursor(1), ref new SNote(5));
	//	testText->addSymbol(ref new Cursor(2), ref new SNote(7));
	//	testText->addSymbol(ref new Cursor(3), ref new SNote(8));
	//	testText->addSymbol(ref new Cursor(4), ref new SNote(10));
	//	testText->addSymbol(ref new Cursor(5), ref new SNote(12));
	//	testText->addSymbol(ref new Cursor(6), ref new SNote(14));
	//	testText->addSymbol(ref new Cursor(7), ref new SNote(15));
	//	test->texts->Append(testText);
	//});
}

void StrokeEditor::ManageInstrumentsDialog::ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
	auto selectedInstrument = (Instrument^)InstrumentList->SelectedItem;
	auto selectedPreset = (SketchMusic::SFReader::SFPreset^) PresetList->SelectedItem;

	if (selectedInstrument && selectedPreset)
	{
		Selected = ref new Instrument(selectedInstrument->_name, selectedInstrument->FileName, selectedPreset->name);
	}
}

void StrokeEditor::ManageInstrumentsDialog::ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs^ args)
{
}


void StrokeEditor::ManageInstrumentsDialog::ContentDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog^ sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs^ args)
{
	Instruments = ref new Platform::Collections::Vector<SketchMusic::Instrument^>;
	Presets = ref new Platform::Collections::Vector<SketchMusic::SFReader::SFPreset^>;
	Texts = ref new Platform::Collections::Vector<SketchMusic::Instrument^>;

	// обрабатываем входные данные
	auto data = dynamic_cast<SketchMusic::CompositionData^>(this->DataContext);
	if (data)
	{
		// не трогаем переданные нам данные; создаём у себя аналогичные тексты, чтобы проводить манипуляции с ними
		for (auto&& text : data->texts)
		{
			Texts->Append(ref new Instrument(text->instrument));
		}
	}

	// получение доступных файлов
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]()
	{
		auto baseFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		
		auto getFiles = create_task([=]
		{
			return create_task(baseFolder->GetFolderAsync("SketchMusic\\resources\\"));
		}).then([=](task<StorageFolder^> resourceFolderTask)->task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^ >
		{
			auto resourceFolder = resourceFolderTask.get();
			return create_task(resourceFolder->GetFilesAsync());
		}).then([=](task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^> resourceListTask)
		{
			auto list = resourceListTask.get();
			for (auto&& file : list)
			{
				Instruments->Append(ref new Instrument(file->Name, file->Path, nullptr));
			}
		}).then([=]
		{
			this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]()
			{
				if (InstrumentList)
				{
					InstrumentList->ItemsSource = Instruments;
				}
			}));
		});

		// считываем файлы из дополнительной папки, если таковая указана
		// Плохо, но я пока не понял, как повторно использовать продолжения.
		// Видимо, нужно их оформить в виде самостоятельных тасков и вызывать в продолжении
		Windows::Storage::ApplicationDataContainer^ localSettings =
			Windows::Storage::ApplicationData::Current->LocalSettings;
		if (localSettings->Values->HasKey("sf2_path"))
		{
			auto path = (Platform::String^)localSettings->Values->Lookup("sf2_path");
			create_task(baseFolder->GetFolderFromPathAsync(path))
				.then([=](task<StorageFolder^> resourceFolderTask)->task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^ >
			{
				auto resourceFolder = resourceFolderTask.get();
				return create_task(resourceFolder->GetFilesAsync());
			}).then([=](task < Windows::Foundation::Collections::IVectorView < Windows::Storage::StorageFile^ > ^> resourceListTask)
			{
				auto list = resourceListTask.get();
				for (auto&& file : list)
				{
					auto instr = ref new Instrument(file->Name, file->Path, nullptr);
					Instruments->Append(instr);
				}
			}).then([=]
			{
				this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([=]()
				{
					if (InstrumentList)
					{
						InstrumentList->ItemsSource = Instruments;
					}
				}));
			});
		}

		((App^)App::Current)->WriteToDebugFile("Составляем список инструментов");
	}));
}


void StrokeEditor::ManageInstrumentsDialog::InstrumentList_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	// получение пресетов для выбранного инструмента
	auto instr = dynamic_cast<Instrument^>(e->ClickedItem);
	if (instr)
	{
		// cбрасываем список, чтобы во время выполнения асинхронного чтения не возникало желания ткнуть в него
		//PresetList->Items->Clear();
		// спрашиваем у плеера, загружен ли такой инструмент и есть ли у него пресеты
		// - выводим окошко со списком доступных пресетов
		this->Dispatcher->RunAsync(
			Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([=]() {

			auto init = concurrency::create_task([=]
			{
				// такая обёртка, если асинхронно будем читать данные, если они ешё не созданы
				return ((App^)App::Current)->_player->GetSFData(instr);
			}).then([=](SketchMusic::SFReader::SFData^ data)->void {
				this->Dispatcher->RunAsync(
					Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([=]() {

					if (data == nullptr) return;

					PresetList->ItemsSource = data->presets;
					
				}));
			});
		}));
	}
}


void StrokeEditor::ManageInstrumentsDialog::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//if (((App^)Application::Current)->_player->_state != SketchMusic::Player::PlayerState::STOP)
	//{
	//	((App^)Application::Current)->_player->_state = SketchMusic::Player::PlayerState::STOP;
	//	return;
	//}

	auto selectedInstrument = (SketchMusic::Instrument^)InstrumentList->SelectedItem;
	auto selectedPreset = (SketchMusic::SFReader::SFPreset^) PresetList->SelectedItem;

	if (selectedInstrument && selectedPreset)
	{
		auto instrument = ref new SketchMusic::Instrument(selectedInstrument->_name,selectedInstrument->FileName, selectedPreset->name);
		((App^)Application::Current)->_player->playSingleNote(ref new SketchMusic::SNote(0), instrument,500, nullptr);
	}
}
