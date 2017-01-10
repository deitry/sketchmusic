﻿//
// CompositionEditorPage.xaml.h
// Объявление класса CompositionEditorPage
//

#pragma once

#include "CompositionEditorPage.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class CompositionEditorPage sealed
	{
	public:
		CompositionEditorPage();
	private:
		void GoBackBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}