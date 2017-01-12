//
// CompositionView.xaml.h
// Declaration of the CompositionView class
//

#pragma once

#include "SketchMusic\view\CompositionView.g.h"

using namespace Windows::Foundation::Collections;
using namespace SketchMusic;


// Отображение формы композиции на основе данных о символах
[Windows::Foundation::Metadata::WebHostHidden]
public ref class SketchMusic::View::CompositionView sealed
{
private:
	IObservableVector<PositionedSymbol^>^ m_parts;

public:
	CompositionView();

	property IObservableVector<PositionedSymbol^>^ Parts
	{
		IObservableVector<PositionedSymbol^>^ get() { return m_parts; }
	}

	void SetParts(IObservableVector<PositionedSymbol^>^ parts);
	void UpdateSize();
	void UpdateView();
	void UpdatePartPos();
	void OnVectorChanged(IObservableVector<PositionedSymbol^>^ sender, IVectorChangedEventArgs^ args);
};
