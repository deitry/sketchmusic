//
// IdeaGrid.xaml.cpp
// Implementation of the IdeaGrid class
//

#include "pch.h"
#include "IdeaGrid.xaml.h"
#include "dialogs\AddIdeaDialog.xaml.h"

using namespace StrokeEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Shapes;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

IdeaGrid::IdeaGrid()
{
	InitializeComponent();
	Lines = ref new Platform::Collections::Vector<Line^>;
	IdeasCtrl = ref new Platform::Collections::Vector<ContentControl^>;
}

void StrokeEditor::IdeaGrid::CreateGrid()
{
	// предполагаем, что ширина нашего контрола актуальна
	// удаляем старые линии - TODO : чтобы не пересоздавать, можно те что есть отрисовывать в нужных местах
	Lines->Clear();
	GridCanvas->Children->Clear();

	if (Parts == nullptr) return;

	// Для каждой части нарисовать вертикальную линию
	int cur = 0;
	for (auto&& part : Parts)
	{
		auto line = ref new Line;
		//binding y с Height? или вручную менять в Update?
		line->X1 = 0; line->X2 = 0;
		line->Y1 = 0;
		line->Y2 = GridCanvas->ActualHeight;

		line->Stroke = (SolidColorBrush^)this->Resources->Lookup("PrimaryLineBrush");
		line->StrokeThickness = 3;
		
		auto pos = ref new Cursor(cur);
		line->DataContext = pos;
		
		Lines->Append(line);
		GridCanvas->Children->Append(line);

		auto point = GetCoordinatsOfPosition(pos);
		GridCanvas->SetLeft(line, point.X);
		GridCanvas->SetTop(line, 0);

		cur += part->Length;
	}

	// отрисовываем горизонтальные линии для слоёв
	for (int i = 0; i < 9; i++)
	{
		auto line = ref new Line;
		//binding y с Height? или вручную менять в Update?
		line->Y1 = 0; line->Y2 = 0;
		line->X1 = 0;
		line->X2 = GridCanvas->ActualWidth;

		line->Stroke = (SolidColorBrush^)this->Resources->Lookup("SecondaryLineBrush");
		line->StrokeThickness = 1;

		line->DataContext = i;
		Lines->Append(line);
		GridCanvas->Children->Append(line);

		GridCanvas->SetTop(line, i*(double)this->Resources->Lookup("LayerHeight"));
		GridCanvas->SetLeft(line, 0);
	}

	if (IdeasCtrl)
	{
		IdeasCtrl->Clear();
		IdeaCanvas->Children->Clear();
		if (Ideas)
		{
			for (auto&& idea : Ideas)
			{
				AddIdeaView(idea);
			}
		}
	}
}

void StrokeEditor::IdeaGrid::UpdateGrid()
{
	// В зависимости от суммарной длины/ длины каждой части в отдельности (/ тактов?)
	// нарисовать полупрозрачные линии, к которым можно будет "привязываться" ~ мелкая сетка

	for (auto&& line : Lines)
	{
		auto pos = dynamic_cast<Cursor^>(line->DataContext);
		if (pos)
		{
			line->Y2 = GridCanvas->ActualHeight;
			// если есть положение - полагаем, что у нас 
			auto point = GetCoordinatsOfPosition(pos);
			GridCanvas->SetLeft(line, point.X);
			GridCanvas->SetTop(line, 0);
		}
		else
		{
			int layer = (int)line->DataContext;
			line->X2 = GridCanvas->ActualWidth;
			GridCanvas->SetLeft(line, 0);
			GridCanvas->SetTop(line, layer*(double)this->Resources->Lookup("LayerHeight"));
		}
	}

	if (IdeasCtrl)
	{
		for (auto&& ctrl : IdeasCtrl)
		{
			auto idea = (PositionedIdea^)ctrl->DataContext;
			auto point = GetCoordinatsOfPosition(idea->Pos, idea->Layer);
			IdeaCanvas->SetLeft(ctrl, point.X);
			IdeaCanvas->SetTop(ctrl, point.Y);
		}
	}
}


void StrokeEditor::IdeaGrid::Parts::set(IObservableVector<PartDefinition^>^ parts)
{
	if (m_parts == parts) return;
	if (parts)
	{
		m_parts = parts;
		CreateGrid();
		m_parts->VectorChanged+= ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PartDefinition ^>(this,
			&StrokeEditor::IdeaGrid::OnVectorChanged);
	}
}

void StrokeEditor::IdeaGrid::OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs^ args)
{
	switch (args->CollectionChange)
	{
	case CollectionChange::ItemRemoved:
	case CollectionChange::ItemInserted:
		UpdateGrid();
		break;
	}
}
// Обновляем отрисовку частей - в частности, положение и размеры
//void StrokeEditor::IdeaGrid::UpdateIdeas()
//{
//	if (IdeasCtrl == nullptr) return;
//
//	// проходимся по списку частей
//	// - корректируем положение
//	for (auto&& ctrl : IdeasCtrl)
//	{
//		auto idea = (PositionedIdea^)ctrl->DataContext;
//		auto point = GetCoordinatsOfPosition(idea->Pos, idea->Layer);
//		IdeaCanvas->SetLeft(ctrl, point.X);
//		IdeaCanvas->SetTop(ctrl, point.Y);
//	}
//}

// Создаём визуальное отображение для идеи
void StrokeEditor::IdeaGrid::AddIdeaView(PositionedIdea ^ idea)
{
	// создаём графическое представление для данной идеи
	ContentControl^ ctrl = ref new ContentControl;
	ctrl->Style = (Windows::UI::Xaml::Style^) this->Resources->Lookup("PIdeaStyle");
	ctrl->DataContext = idea;
	//ctrl->Width = 50;
	ctrl->Content = "idea";

	IdeasCtrl->Append(ctrl);

	IdeaCanvas->Children->Append(ctrl);
	auto point = GetCoordinatsOfPosition(idea->Pos, idea->Layer);
	IdeaCanvas->SetLeft(ctrl, point.X);
	IdeaCanvas->SetTop(ctrl, point.Y);
	// не забываем добавить хендлеры для 
	// - нажатия на объект
	// - вызова контекстного меню
	// - драг & дроп ?
}

// удаляем визуальное отображение
void StrokeEditor::IdeaGrid::DeleteIdeaView(PositionedIdea ^ idea)
{
	
}


Windows::Foundation::Point StrokeEditor::IdeaGrid::GetCoordinatsOfPosition(Cursor ^ pos, int layer)
{
	// исходя из ширины контрола и длины композиции вычисляем положение
	int total = 0;
	for (auto&& part : Parts)
	{
		total += part->Length;
	}

	float newX = GridCanvas->ActualWidth * pos->Beat / total;
	float newY = layer*(double)this->Resources->Lookup("LayerHeight");

	return Windows::Foundation::Point(newX, newY);
}

Cursor ^ StrokeEditor::IdeaGrid::GetPositionOfPoint(Windows::Foundation::Point point)
{
	int total = 0;
	for (auto&& part : Parts)
	{
		total += part->Length;
	}

	int newpos = (int)(point.X * total / GridCanvas->ActualWidth);
	return ref new Cursor(newpos);
}

void StrokeEditor::IdeaGrid::CreateNewIdea(Windows::Foundation::Point point)
{
	auto pos = GetPositionOfPoint(point);
	int layer = point.Y / (double)this->Resources->Lookup("LayerHeight");

	auto dialog = ref new AddIdeaDialog;
	concurrency::create_task(dialog->ShowAsync())
		.then([=](ContentDialogResult result) {
		if (result == ContentDialogResult::Primary)
		{
			auto idea = dialog->NewIdea;
			if (idea)
			{
				// TODO - ставим флаг о том, что вводится новый элемент, чтобы можно было дальнейшим перемещением установить длину
				// - для начала вставляем сразу, не даём выбора
				auto posIdea = ref new PositionedIdea(pos, layer, idea);
				Ideas->Append(posIdea);
				AddIdeaView(posIdea);
			}
		}
	});
}


void StrokeEditor::IdeaGrid::MainCanvas_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	// если перетаскиваемый объект - идея
	// - фиксируем положение (SketchMusic::Cursor)
	// - длину принимаем по умолчанию - последний символ в контенте идеи
	// - создаём графический объект
}


void StrokeEditor::IdeaGrid::MainCanvas_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	// не пропускаем тач
	if (e->Pointer->PointerDeviceType != Windows::Devices::Input::PointerDeviceType::Touch)
	{// - фиксируем положение
	// - - вычисляем Cursor по X и слой по Y
		auto point = e->GetCurrentPoint(GridCanvas)->Position;
		CreateNewIdea(point);
	}

}


void StrokeEditor::IdeaGrid::MainCanvas_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void StrokeEditor::IdeaGrid::UserControl_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	UpdateGrid();
	//UpdateIdeas();
}


void StrokeEditor::IdeaGrid::IdeaCanvas_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e)
{
	// пропускаем только нажатие тачем
	if ((e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen))
		return;
}


void StrokeEditor::IdeaGrid::IdeaCanvas_RightTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs^ e)
{
	// не пропускаем тач
	if (e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;
}


void StrokeEditor::IdeaGrid::IdeaCanvas_DoubleTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs^ e)
{
	// пропускаем только нажатие тачем
	if ((e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen))
		return;

	auto point = e->GetPosition(GridCanvas);
	CreateNewIdea(point);
}
