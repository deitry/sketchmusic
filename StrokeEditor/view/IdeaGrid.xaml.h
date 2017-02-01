//
// IdeaGrid.xaml.h
// Declaration of the IdeaGrid class
//

#pragma once

#include "view\IdeaGrid.g.h"

using namespace SketchMusic;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Shapes;

namespace StrokeEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class IdeaGrid sealed
	{
	private:
		IObservableVector<PartDefinition^>^ m_parts;

	public:
		IdeaGrid();

		property Composition^ Project;
		property IObservableVector<PartDefinition^>^ Parts
		{
			IObservableVector<PartDefinition^>^ get() { return m_parts; }
			void set(IObservableVector<PartDefinition^>^ parts)
			{ 
				if (m_parts == parts) return;
				if (parts)
				{
					CreateGrid();
				}
				m_parts = parts;
			}
		}

		void CreateGrid();	// создаём сетку
		void UpdateGrid();	// перерисовываем сетку
		void UpdateIdeas();	// отрисовываем на канвасе привязанные идеи
		void AddIdeaView(PositionedIdea^ idea);	// создаём визуальное отображение для идеи
		void DeleteIdeaView(PositionedIdea^ idea);	// удаляем визуальное отображение

	protected:
		Windows::Foundation::Point GetCoordinatsOfPosition(Cursor^ pos, int layer = 0);

	private:
		void MainCanvas_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
		void MainCanvas_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void MainCanvas_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);

		IVector<Line^>^ Lines;
		void UserControl_SizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
	};
}
