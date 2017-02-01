//
// IdeaGrid.xaml.cpp
// Implementation of the IdeaGrid class
//

#include "pch.h"
#include "IdeaGrid.xaml.h"

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
}

void StrokeEditor::IdeaGrid::CreateGrid()
{
	// предполагаем, что ширина нашего контрола актуальна
	// удаляем старые линии - TODO : чтобы не пересоздавать, можно те что есть отрисовывать в нужных местах
	Lines->Clear();
	MainCanvas->Children->Clear();

	if (Parts == nullptr) return;

	// Для каждой части нарисовать вертикальную линию
	int cur = 0;
	for (auto&& part : Parts)
	{
		auto line = ref new Line;
		//binding y с Height? или вручную менять в Update?
		line->X1 = 0; line->X2 = 0;
		line->Y1 = 0;
		line->Y2 = MainCanvas->ActualHeight;

		line->Stroke = (SolidColorBrush^)this->Resources->Lookup("PrimaryLineBrush");
		line->StrokeThickness = 3;
		
		auto pos = ref new Cursor(cur);
		line->DataContext = pos;
		
		Lines->Append(line);
		MainCanvas->Children->Append(line);

		auto point = GetCoordinatsOfPosition(pos);
		MainCanvas->SetLeft(line, point.X);
		MainCanvas->SetTop(line, 0);

		cur += part->Length;
	}

	// отрисовываем горизонтальные линии для слоёв
	for (int i = 0; i < 9; i++)
	{
		auto line = ref new Line;
		//binding y с Height? или вручную менять в Update?
		line->Y1 = 0; line->Y2 = 0;
		line->X1 = 0;
		line->X2 = MainCanvas->ActualWidth;

		line->Stroke = (SolidColorBrush^)this->Resources->Lookup("SecondaryLineBrush");
		line->StrokeThickness = 1;
		
		line->DataContext = i;
		Lines->Append(line);
		MainCanvas->Children->Append(line);

		MainCanvas->SetTop(line, i*(double)this->Resources->Lookup("LayerHeight"));
		MainCanvas->SetLeft(line, 0);
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
			line->Y2 = MainCanvas->ActualHeight;
			// если есть положение - полагаем, что у нас 
			auto point = GetCoordinatsOfPosition(pos);
			MainCanvas->SetLeft(line, point.X);
			MainCanvas->SetTop(line, 0);
		}
		else
		{
			int layer = (int)line->DataContext;
			line->X2 = MainCanvas->ActualWidth;
			MainCanvas->SetLeft(line, 0);
			MainCanvas->SetTop(line, layer*(double)this->Resources->Lookup("LayerHeight"));
		}
	}
}

// Обновляем отрисовку частей - в частности, положение и размеры
void StrokeEditor::IdeaGrid::UpdateIdeas()
{
	// проходимся по списку частей
	// - если есть положение, создаём визуальный объект
}

// Создаём визуальное отображение для идеи
void StrokeEditor::IdeaGrid::AddIdeaView(PositionedIdea ^ idea)
{
	// создаём графическое представление для данной идеи

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

	float newX = MainCanvas->ActualWidth * pos->Beat / total;
	float newY = layer*(double)this->Resources->Lookup("LayerHeight");

	return Windows::Foundation::Point(newX, newY);
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
	// - фиксируем положение
	// - ставим флаг о том, что вводится новый элемент
	// - 
}


void StrokeEditor::IdeaGrid::MainCanvas_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void StrokeEditor::IdeaGrid::UserControl_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	UpdateGrid();
	UpdateIdeas();
}
