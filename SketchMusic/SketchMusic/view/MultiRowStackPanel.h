//
// MultiRowStackPanel.h
// Объявление класса MultiRowStackPanel.
//

#pragma once

#include "../SketchMusic.h"

public ref class SketchMusic::View::MultiRowStackPanel sealed : public Windows::UI::Xaml::Controls::Control
{
public:
	MultiRowStackPanel();

	void NewLine(int i) {}	// вставить новую линию
};

