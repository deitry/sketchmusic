//
// ChordView.xaml.h
// Declaration of the ChordView class
//

#pragma once

#include "..\SketchMusic.h"
#include "../base/Cursor.h"
#include "SketchMusic\view\ChordView.g.h"

using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace SketchMusic;

public ref class SketchMusic::View::TestData sealed
{
public:
	property IVector<PositionedSymbol^>^ TestPSymCollection
	{
		IVector<PositionedSymbol^>^ get() {
			auto notes = ref new Vector<PositionedSymbol^>;
			notes->Append(ref new PositionedSymbol(ref new SketchMusic::Cursor(), ref new SNote(0)));
			notes->Append(ref new PositionedSymbol(ref new SketchMusic::Cursor(), ref new SNote(3)));
			notes->Append(ref new PositionedSymbol(ref new SketchMusic::Cursor(), ref new SNote(7)));
			notes->Append(ref new PositionedSymbol(ref new SketchMusic::Cursor(), ref new SNote(11)));
			return notes;
		}
	}
};

[Windows::Foundation::Metadata::WebHostHidden]
public ref class SketchMusic::View::ChordView sealed
{
private:
	void SetNoteOnCanvas(Windows::UI::Xaml::Controls::ContentControl^ ctrl);
	double GetOffsetY(ISymbol ^ sym);

	Windows::Foundation::Collections::IVector<SketchMusic::PositionedSymbol^>^ Notes;

public:
	ChordView();
	
	void SetNotes(Windows::Foundation::Collections::IObservableVector<SketchMusic::PositionedSymbol^>^ notes);
	
};
