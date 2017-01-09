//
// ChordView.xaml.cpp
// Implementation of the ChordView class
//

#include "pch.h"
#include "ChordView.xaml.h"

using namespace SketchMusic;

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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

SketchMusic::View::ChordView::ChordView()
{
	InitializeComponent();
}

void SketchMusic::View::ChordView::SetNotes(Windows::Foundation::Collections::IObservableVector<PositionedSymbol^>^ notes)
{
	_ItemsControl->ItemsSource = notes;
	/*for (auto psym : notes)
	{
		// создаём объект и размещаем его на канвасе
		Windows::UI::Xaml::Controls::ContentControl^ ctrl = ref new Windows::UI::Xaml::Controls::ContentControl;
		ctrl->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(this->Resources->Lookup("SymbolControlStyle"));
		ctrl->Content = psym;
		
		// обработчики нажатия
		//bt->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
		//bt->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
		//bt->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);

		_canvas->Children->Append(ctrl);
		
		//Binding^ canvasY = ref new Binding();
		//canvasY->Source = ctrl->Content;
		//canvasY->Path = ref new PropertyPath("Canvas.Top");
		//canvasY->Converter = dynamic_cast<SketchMusic::View::PSymbolToVerticalPosConverter^>(this->Resources->Lookup("symbol2ypos")); //Windows::UI::Xaml::Data::IValueConverter^
		//canvasY->ConverterParameter = ctrl->Height;
		//_canvas->SetBinding(Canvas::TopProperty, canvasY);


		//auto transform = ctrl->TransformToVisual(_canvas);
		//Point basePoint = Point(0, 0);
		//auto point = transform->TransformPoint(basePoint);

		//double offsetY = GetOffsetY(psym->_sym);
		//_canvas->SetLeft(ctrl, 50);
		//_canvas->SetTop(ctrl, point.Y + offsetY);
	}*/
}

double SketchMusic::View::ChordView::GetOffsetY(ISymbol ^ sym)
{
	double offsetY = 0;
	double RowHeight = (double)this->Resources->Lookup("RowHeight");
	
	// если обрабатываемый символ - нота
	INote^ inote = dynamic_cast<INote^>(sym);
	if (inote)
	{
		
		offsetY = inote->_val >= 0 ?
			-abs(inote->_val) % 12 :
			-12 * ((inote->_val % 12) != 0) + abs(inote->_val) % 12;
		offsetY *= RowHeight / 12;
		offsetY += RowHeight - 5;
	}
	else {
		STempo^ tempo = dynamic_cast<STempo^>(sym);
		if (tempo)
		{
			offsetY = -0.6*RowHeight;
		}
	}
	return offsetY;
}

void SketchMusic::View::ChordView::SetNoteOnCanvas(Windows::UI::Xaml::Controls::ContentControl ^ ctrl)
{
}
