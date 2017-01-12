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
	auto parts = ref new Platform::Collections::Vector<PositionedSymbol^>;
	DynamicCategory cat;
	cat.quite = true;
	cat.regular = true;
	parts->Append(ref new PositionedSymbol(ref new Cursor(0),ref new SNewPart("A",0,cat)));
	parts->Append(ref new PositionedSymbol(ref new Cursor(2), ref new SNewPart("B",0,cat)));
	parts->Append(ref new PositionedSymbol(ref new Cursor(4), ref new SNewPart("A",1,cat)));
	parts->Append(ref new PositionedSymbol(ref new Cursor(8), ref new SNewPart("B",1,cat)));

	InitializeComponent();
	SetParts(parts);

	m_parts->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<SketchMusic::PositionedSymbol ^>(this, 
		&SketchMusic::View::CompositionView::OnVectorChanged);
}

void SketchMusic::View::CompositionView::SetParts(IObservableVector<PositionedSymbol^>^ parts)
{
	m_parts = parts;
	PositionedSymbol^ prev = nullptr;
	for (auto&& part : m_parts)
	{
		// вычисляем длину частей и номера согласно категориям
		// TODO : Из категорий составляем список, запоминаем
		
		if (prev)
		{
			auto prevPart = dynamic_cast<SNewPart^>(prev->_sym);
			if (prevPart)
			{
				prevPart->length = part->_pos->getBeat() - prev->_pos->getBeat();
			}
		}
		prev = part;
	}
	UpdateView();
}

void SketchMusic::View::CompositionView::OnVectorChanged(IObservableVector<PositionedSymbol^>^ sender, IVectorChangedEventArgs^ args)
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
	ContentControl^ prev = nullptr;
	int prevBeat = 0;
	int endBeat = m_parts->GetAt(Parts->Size-1)->_pos->getBeat();

	auto child = CompositionPanel->Children->First();
	auto part = m_parts->First();
	
	while (part->HasCurrent)
	{
		auto ctrl = dynamic_cast<ContentControl^>(static_cast<Object^>(child->Current));
		//auto part = dynamic_cast<PositionedSymbol^>(static_cast<Object^>(child));

		if (prev)
		{
			//double relWid = ;
			prev->Width = this->Width * (part->Current->_pos->getBeat() - prevBeat) / endBeat;
		}
		prev = ctrl;
		prevBeat = part->Current->_pos->getBeat();
		child->MoveNext();
		part->MoveNext();
	}
	CompositionPanel->UpdateLayout();
}

void SketchMusic::View::CompositionView::UpdateView()
{
	CompositionPanel->Children->Clear();
	
	UpdatePartPos();

	auto end = m_parts->GetAt(m_parts->Size - 1);	// быстрая ссылка на последний элемент
	for (auto&& part : m_parts)
	{
		if (part == end) break;

		auto ctrl = ref new ContentControl;
		ctrl->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(this->Resources->Lookup("CompositionPartStyle"));
		ctrl->DataContext = part;
		CompositionPanel->Children->Append(ctrl);
	}

	UpdateSize();
}

void SketchMusic::View::CompositionView::UpdatePartPos()
{
	int curPos = 0;
	for (auto&& part : m_parts)
	{
		// вычисляем новые положения posytionedSymbol исходя из вычисленной ранее длины
		auto current = dynamic_cast<SNewPart^>(part->_sym);
		part->_pos->setPos(curPos);
		if (current)
			curPos += current->length;
	}
}
