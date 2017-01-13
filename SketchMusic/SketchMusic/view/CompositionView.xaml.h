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
	IObservableVector<PartDefinition^>^ m_parts;

public:
	CompositionView();

	property IObservableVector<PartDefinition^>^ Parts
	{
		IObservableVector<PartDefinition^>^ get() { return m_parts; }
	}

	void SetParts(IObservableVector<PartDefinition^>^ parts);
	void UpdateSize();
	void UpdateView();
	void OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs^ args);
};
