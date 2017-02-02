//
// IdeaGrid.xaml.h
// Declaration of the IdeaGrid class
//

#pragma once

#include "view\IdeaGrid.g.h"

using namespace SketchMusic;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Shapes;
using namespace Windows::UI::Xaml::Controls;

namespace StrokeEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class IdeaGrid sealed
	{
	private:
		IObservableVector<PartDefinition^>^ m_parts;
		IObservableVector<ContentControl^>^ IdeasCtrl;

		ContentControl^ _Selected;
		ContentControl^ _Dragged;
		Point _DraggedOffset;

	public:
		IdeaGrid();

		//property Composition^ Project;
		property IObservableVector<PositionedIdea^>^ Ideas;
		property IObservableVector<PartDefinition^>^ Parts
		{
			IObservableVector<PartDefinition^>^ get() { return m_parts; }
			void set(IObservableVector<PartDefinition^>^ parts);
		}

		void CreateGrid();	// создаём сетку
		void UpdateGrid();	// перерисовываем сетку
		void OnVectorChanged(IObservableVector<PartDefinition^>^ sender, IVectorChangedEventArgs ^ args);
		//void UpdateIdeas();	// отрисовываем на канвасе привязанные идеи
		void AddIdeaView(PositionedIdea^ idea);	// создаём визуальное отображение для идеи
		void DeleteIdeaView(PositionedIdea^ idea);	// удаляем визуальное отображение

	protected:
		Windows::Foundation::Point GetCoordinatsOfPosition(Cursor^ pos, int layer = 0);
		Cursor^ GetPositionOfPoint(Point point);
		void CreateNewIdea(Point point);
		//void SetIdeaOnCanvas(PositionedIdea^ idea);

	private:
		IVector<Line^>^ Lines;

		void OpenContextMenu(Platform::Object^ sender, Windows::Foundation::Point point);

		void UserControl_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void IdeaCanvas_Holding(Platform::Object^ sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs^ e);
		void IdeaCanvas_RightTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs^ e);
		void IdeaCanvas_DoubleTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs^ e);
		void OnPointerPressed(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
		void OnHolding(Platform::Object ^sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs ^e);
		void OnRightTapped(Platform::Object ^sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs ^e);
		void OnPointerMoved(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
		void OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
		void EditIdeaBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteIdeaBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void IdeaContextMenu_Closed(Platform::Object^ sender, Platform::Object^ e);
	};
}
