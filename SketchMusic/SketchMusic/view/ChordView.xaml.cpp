//
// ChordView.xaml.cpp
// Implementation of the ChordView class
//

#include "pch.h"
#include "ChordView.xaml.h"
#include "../base/STempo.h"

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
	Notes = notes;
	_ItemsControl->ItemsSource = Notes;
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
