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
	PartDefinition^ p1 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(0), ref new SNewPart("A", DynamicCategory::quite)));
	p1->length = 2;
	PartDefinition^ p2 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(2), ref new SNewPart("B", DynamicCategory::harder)));
	p2->length = 2;
	PartDefinition^ p3 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(4), ref new SNewPart("A", DynamicCategory::regular)));
	p3->length = 5; p3->number = 1;
	PartDefinition^ p4 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(9), ref new SNewPart("B", DynamicCategory::fast)));
	p4->length = 1; p4->number = 1;

	parts->Append(p1);
	parts->Append(p2);
	parts->Append(p3);
	parts->Append(p4);

	InitializeComponent();
	SetParts(parts);

	m_parts->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PartDefinition ^>(this,
		&SketchMusic::View::CompositionView::OnVectorChanged);
}

void SketchMusic::View::CompositionView::SetParts(IObservableVector<PartDefinition^>^ parts)
{
	m_parts = parts;
	PartDefinition^ prev = nullptr;
	
	UpdateView();
}

void SketchMusic::View::CompositionView::OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs^ args)
{
	// обновляем список категорий и количество вхождений
	Categories->Clear();
	for (auto&& part : sender)
	{
		auto cat = part->original->category;
		if (cat == nullptr || cat == "")
		{
			part->number = 0;
			continue;
		}

		if (Categories->HasKey(cat))
		{
			auto val = Categories->Lookup(cat);
			Categories->Insert(cat, val+1);
			part->number = val + 1;
		}
		else
		{
			Categories->Insert(cat, 0);
			part->number = 0;
		}
	}

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
	unsigned int ind;
	if (m_parts->IndexOf(part, &ind))
	{
		if (part != nullptr)
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
		totalLength += p->length;
	}

	for (auto&& child : CompositionPanel->Children)
	{
		auto ctrl = dynamic_cast<FrameworkElement^>(static_cast<Object^>(child));
		if (ctrl == nullptr) continue;
		auto part = dynamic_cast<PartDefinition^>(ctrl->DataContext);

		ctrl->Width = this->Width * part->length / totalLength;
	}
	CompositionPanel->UpdateLayout();
}

void SketchMusic::View::CompositionView::UpdateView()
{
	CompositionPanel->Children->Clear();
	
	auto end = m_parts->GetAt(m_parts->Size - 1);	// быстрая ссылка на последний элемент
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

