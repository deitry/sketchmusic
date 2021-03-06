﻿//
// MainMenuPage.xaml.h
// Объявление класса MainMenuPage
//

#pragma once

#include "MainMenuPage.g.h"

namespace StrokeEditor
{
	/// <summary>
	/// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class MainMenuPage sealed
	{
	public:
		MainMenuPage();
	private:
		void OpenLibraryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AboutBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OpenCompositionLibraryBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CreateCompositionBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CreateIdeaBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void HelpBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SettingsBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
