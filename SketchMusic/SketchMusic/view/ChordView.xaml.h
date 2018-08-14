//
// ChordView.xaml.h
// Declaration of the ChordView class
//

#pragma once

#include "../SketchMusic.h"
#include "../base/Cursor.h"
#include "../base/Text.h"
#include "../base/SNote.h"
#include "SketchMusic/view/ChordView.g.h"

using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace SketchMusic;

[Windows::Foundation::Metadata::WebHostHidden]
public ref class SketchMusic::View::TestData sealed
{
public:
	property IVector<PositionedSymbol^>^ TestPSymCollection
	{
		IVector<PositionedSymbol^>^ get() {
			auto notes = ref new Vector<PositionedSymbol^>;
			notes->Append(ref new PositionedSymbol(ref new Cursor, ref new SNote(0)));
			notes->Append(ref new PositionedSymbol(ref new Cursor, ref new SNote(3)));
			notes->Append(ref new PositionedSymbol(ref new Cursor, ref new SNote(7)));
			notes->Append(ref new PositionedSymbol(ref new Cursor, ref new SNote(11)));
			return notes;
		}
	}

	static Text^ CreateTestText(int length)
	{
		Text^ text = ref new Text;
		for (int i = 0; i < length; i++)
		{
			text->addSymbol(ref new PositionedSymbol(ref new Cursor(i), ref new SNote(i % 12)));
			if (i%2)
				text->addSymbol(ref new PositionedSymbol(ref new Cursor(i), ref new SNote(i % 12 + 5)));
		}
		return text;
	}
};

[Windows::Foundation::Metadata::WebHostHidden]
public ref class SketchMusic::View::ChordView sealed
{
private:
	void SetNoteOnCanvas(Windows::UI::Xaml::Controls::ContentControl^ ctrl);
	double GetOffsetY(ISymbol ^ sym);
	Windows::Foundation::Collections::IObservableVector<SketchMusic::PositionedSymbol^>^ Notes;

public:
	ChordView();
	
	void SetNotes(Windows::Foundation::Collections::IObservableVector<SketchMusic::PositionedSymbol^>^ notes);
	Windows::Foundation::Collections::IObservableVector<SketchMusic::PositionedSymbol^>^ GetNotes() { return Notes; }
};
