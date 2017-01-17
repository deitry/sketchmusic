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
	auto parts = ref new Platform::Collections::Vector<PartDefinition^>;
	DynamicCategory cat;
	cat.quite = true;
	cat.regular = true;
	PartDefinition^ p1 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(0), ref new SNewPart("A", cat)));
	p1->length = 2;
	PartDefinition^ p2 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(2), ref new SNewPart("B", cat)));
	p2->length = 2;
	PartDefinition^ p3 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(4), ref new SNewPart("A", cat)));
	p3->length = 5; p3->number = 1;
	PartDefinition^ p4 = ref new PartDefinition(ref new PositionedSymbol(ref new Cursor(9), ref new SNewPart("B", cat)));
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
	switch (args->CollectionChange)
	{
	case CollectionChange::ItemRemoved:
		break;
	case CollectionChange::ItemInserted:
		UpdateView();
		break;
	}
}

void SketchMusic::View::CompositionView::UpdateSize()
{
	int endBeat = m_parts->GetAt(Parts->Size-1)->originalPos->Beat;

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
	}

	UpdateSize();
}

