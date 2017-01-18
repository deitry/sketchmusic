//
// CompositionView.xaml.cpp
// Implementation of the CompositionView class
//

#include "pch.h"
#include "CompositionView.xaml.h"

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

CompositionView::CompositionView()
{
	Categories = ref new Platform::Collections::Map<String^, int>;
	auto parts = ref new Platform::Collections::Vector<PartDefinition^>;
	//PartDefinition^ p1 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(0), ref new SNewPart("A", DynamicCategory::quite)));
	//p1->Length = 2; p1->Time = 120 * 2 / 60;
	//PartDefinition^ p2 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(2), ref new SNewPart("B", DynamicCategory::harder)));
	//p2->Length = 2; p2->Time = 120 * 2 / 60;
	//PartDefinition^ p3 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(4), ref new SNewPart("A", DynamicCategory::regular)));
	//p3->Length = 5; p3->Time = 120 * 5 / 60;
	//PartDefinition^ p4 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(9), ref new SNewPart("B", DynamicCategory::fast)));
	//p4->Length = 1;

	//parts->Append(p1);
	//parts->Append(p2);
	//parts->Append(p3);
	//parts->Append(p4);

	InitializeComponent();
	SetParts(parts);

	
}

void SketchMusic::View::CompositionView::SetParts(IObservableVector<PartDefinition^>^ parts)
{
	m_parts = parts;
	m_parts->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PartDefinition ^>(this,
		&SketchMusic::View::CompositionView::OnVectorChanged);
	
	UpdateView();
}

void SketchMusic::View::CompositionView::OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs^ args)
{
	switch (args->CollectionChange)
	{
	case CollectionChange::ItemRemoved:
	case CollectionChange::ItemInserted:
		UpdateView();
		break;
	}
}

void SketchMusic::View::CompositionView::SelectedItem::set(PartDefinition ^ part)
{
	if (part != m_selected)
	{
		for (auto&& child : CompositionPanel->Children)
		{
			auto ctrl = dynamic_cast<ContentControl^>(static_cast<Object^>(child));
			if (ctrl == nullptr) continue;
			auto childData = dynamic_cast<PartDefinition^>(ctrl->DataContext);

			if (childData == part)
			{
				ctrl->Background = reinterpret_cast<Windows::UI::Xaml::Media::SolidColorBrush^>(
					this->Resources->Lookup("SelectedPartBackground"));
			}
			else
			{
				ctrl->Background = nullptr;
			}
		}
		
		m_selected = part;
	}
}

void SketchMusic::View::CompositionView::UpdateSize()
{
	// вычисляем суммарную длину
	int totalLength = 0;
	for (auto&& p : m_parts)
	{
		totalLength += p->Length;
	}

	for (auto&& child : CompositionPanel->Children)
	{
		auto ctrl = dynamic_cast<FrameworkElement^>(static_cast<Object^>(child));
		if (ctrl == nullptr) continue;
		auto part = dynamic_cast<PartDefinition^>(ctrl->DataContext);

		ctrl->Width = this->Width * part->Length / totalLength;
	}
	CompositionPanel->UpdateLayout();
}

void SketchMusic::View::CompositionView::UpdateView()
{
	// обновляем список категорий и количество вхождений
	Categories->Clear();
	for (auto&& part : m_parts)
	{
		auto cat = part->original->category;
		if (cat == nullptr || cat == "")
		{
			part->Number = 0;
			continue;
		}

		if (Categories->HasKey(cat))
		{
			auto val = Categories->Lookup(cat);
			Categories->Insert(cat, val + 1);
			part->Number = val + 1;
		}
		else
		{
			Categories->Insert(cat, 0);
			part->Number = 0;
		}
	}

	CompositionPanel->Children->Clear();
	
	for (auto&& part : m_parts)
	{
		auto ctrl = ref new ContentControl;
		ctrl->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(this->Resources->Lookup("CompositionPartStyle"));
		ctrl->DataContext = part;
		CompositionPanel->Children->Append(ctrl);

		if (part == m_selected)
		{
			ctrl->Background = reinterpret_cast<Windows::UI::Xaml::Media::SolidColorBrush^>(
				this->Resources->Lookup("SelectedPartBackground"));
		}
		else
		{
			ctrl->Background = nullptr;
		}
	}

	UpdateSize();
}

