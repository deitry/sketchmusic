//
// MultiRowStackPanel.cpp
// Реализация класса MultiRowStackPanel.
//

#include "pch.h"
#include "MultiRowStackPanel.h"

using namespace SketchMusic;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

// Документацию по шаблону элемента "Элемент управления на основе шаблона" см. по адресу http://go.microsoft.com/fwlink/?LinkId=234235

SketchMusic::View::MultiRowStackPanel::MultiRowStackPanel()
{
	DefaultStyleKey = "SketchMusic.MultiRowStackPanel";
}
