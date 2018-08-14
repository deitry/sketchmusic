#include "pch.h"
#include "TextRow.xaml.h"
#include "../base/Text.h"
#include "../base/Composition.h"

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

void SketchMusic::View::TextRow::OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
	//InvalidateText();
}

void SketchMusic::View::TextRow::AddBeatItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	// TODO: выполнить команду по добавлению долей

	_maxPos->incBeat();
	if (this->data->texts->Size > 0)
	{
		AllocateSnapPoints(this->GetText(), this->scale, -1);
	}
}

void SketchMusic::View::TextRow::DeleteBeatItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	// TODO: выполнить команду по удалению долей
	
	_maxPos->decBeat();
	if (this->data->texts->Size > 0)
	{
		AllocateSnapPoints(this->GetText(), this->scale, -1);
	}
}

void SketchMusic::View::TextRow::PlaceholderContextMenuOnClosed(Object ^ sender, Platform::Object ^ args)
{
	CurrentHolded->Background = nullptr;
	CurrentHolded = nullptr;
}

void SketchMusic::View::TextRow::InitializePage()
{
	_dict = this->Resources;
	
	BeatWidth = (double)(_dict->Lookup("BeatWidth"));
	PlaceholderWidth = (double)(_dict->Lookup("PlaceholderWidth"));
	RowHeight = (double)(_dict->Lookup("RowHeight"));

	//_mainPanel = (StackPanel^)GetTemplateChild("_mainPanel");
	_mainPanel->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
	_mainPanel->PointerWheelChanged += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerWheelChanged);
	//_mainPanel->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);
	_scrollViewer->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);

	//_canvas = (Canvas^)GetTemplateChild("_canvas");
	_canvas->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
	_canvas->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);

	_snapPoint = ref new ContentControl;
	_snapPoint->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
	_snapPoint->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("SnapPointStyle"));
	_snapPoint->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	_canvas->Children->Append(_snapPoint);


	PlaceholderContextMenu->Closed += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &SketchMusic::View::TextRow::PlaceholderContextMenuOnClosed);
	AddBeatItem->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::TextRow::AddBeatItem_Click);
	DeleteBeatItem->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::TextRow::DeleteBeatItem_Click);
	//_cursor = (ContentControl^)GetTemplateChild("_cursor");

	//_scrollViewer = (ScrollViewer^)GetTemplateChild("_scrollViewer");

	if (this->data->texts->Size > 0)
	{
		AllocateSnapPoints(this->GetText(), 1, -1);
	}

	initialised = 1;
	scale = 2;
	quantize = 1;
}

// масштабирование
void SketchMusic::View::TextRow::OnPointerWheelChanged(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	// ! - тут не обрбабатываются касания двумя пальцами

	auto point = e->GetCurrentPoint(nullptr);	// this
	auto properties = point->Properties;
	auto delta = properties->MouseWheelDelta;

	if (abs(delta) < 120)
		return;

	// на каждые N приращения нужно удваивать количество точек или наоборот делить на два
	int newScale = 2 * delta / abs(delta);
	SetScale(newScale);
}

void SketchMusic::View::TextRow::HighlightSnapPoint(Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	auto point = e->GetCurrentPoint(nullptr)->Position;

	auto listItems = VisualTreeHelper::FindElementsInHostCoordinates(point, _mainPanel);

	// - выделяем долю, которая под курсором
	for (auto item : listItems)
	{
		Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(item);
		if ((ctrl == nullptr) || (((String^)ctrl->Tag) != "phold"))
			continue;

		if (ctrl != _currentSnapPoint)
		{
			_currentSnapPoint = ctrl;
		}
	}

	if (_currentSnapPoint)
	{
		// - подсвечиваем текущую точку привязки
		Point point = GetCoordinatsOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);
		_snapPoint->Visibility = Windows::UI::Xaml::Visibility::Visible;
		Canvas::SetLeft(_snapPoint, point.X);
		Canvas::SetTop(_snapPoint, point.Y - 10);
	}
}

// выбор ноты или чего-нибудь ещё
void SketchMusic::View::TextRow::OnPointerPressed(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	_isPressed = true;
	// TODO : если второй палец 
	// - если нота уже "поднята", ничего не происходит
	// - если нота не "поднята", то замеряем расстояние между пальцами
	// - - если оно меняется, то при достижении определённой "дельты" вызываем SetScale

	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if ((ctrl == nullptr) || ((dynamic_cast<Text^>(ctrl->Tag) != current) && (dynamic_cast<Text^>(ctrl->Tag) != data->ControlText)) )
	{
		// если нажато один раз, но ноты нет, зато есть контрол, то перемещаем указатель в данный контрол
		if (_currentSnapPoint)
		{
			HighlightSnapPoint(e);
			Cursor^ newPos = GetPositionOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);
			SetCursor(newPos);
		}
		return;
	}

	// Если включён ластик - удаляем символ
	if (EraserTool)
	{
		auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(ctrl->DataContext);
		if (((Text^)ctrl->Tag == current || (Text^)ctrl->Tag == data->ControlText) && (psym != nullptr))
		{
			_CommandManager->AddAndExecute(
				DeleteSymCommand,
				ref new SketchMusic::Commands::SymbolHandlerArgs(((Text^)ctrl->Tag),psym));
		}
		return;
	}

	auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(ctrl->DataContext);
	if (psym) SymbolPressed(this, psym);

	_scrollViewer->HorizontalScrollMode = ScrollMode::Disabled;
	_scrollViewer->VerticalScrollMode = ScrollMode::Disabled;
	_lastPoint = e->GetCurrentPoint(_canvas)->Position;
	_lastPoint.X -= Canvas::GetLeft(ctrl);
	_lastPoint.Y -= Canvas::GetTop(ctrl);

	_dragged = ctrl;
	_isMoving = true;
	if (_dict->HasKey("draggedForegroundBrush"))
	{
		_dragged->Foreground = (Windows::UI::Xaml::Media::Brush^)_dict->Lookup("draggedForegroundBrush");
	}
	_canvas->CapturePointer(e->Pointer);
}

// перемещение ноты
void SketchMusic::View::TextRow::OnPointerMoved(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	HighlightSnapPoint(e);

	// Если включён ластик - удаляем символ
	if (EraserTool && (_isPressed) && (_dragged == nullptr))
	{
		ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
		if (ctrl)
		{
			auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(ctrl->DataContext);
			if (psym && ((Text^)ctrl->Tag == current || (Text^)ctrl->Tag == data->ControlText))
			{
				_CommandManager->AddAndExecute(
					DeleteSymCommand,
					ref new SketchMusic::Commands::SymbolHandlerArgs(((Text^)ctrl->Tag), psym));
			}
		}
		return;
	}

	// - связываем выделенный элемент с курсором
	if (_isMoving && _dragged)
	{
		Windows::Foundation::Point pos = e->GetCurrentPoint(reinterpret_cast<Windows::UI::Xaml::Controls::Canvas^>(_canvas))->Position;
		Canvas::SetLeft(_dragged, pos.X - _lastPoint.X); // _current->Width/2
		Canvas::SetTop(_dragged, pos.Y - _lastPoint.Y); // pos.Y - _lastPoint.Y
	}
}

// отпускание ноты или чего-нибудь ещё
void SketchMusic::View::TextRow::OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	_isPressed = false;
	if (_dragged == nullptr)
		return;

	if (_currentSnapPoint != nullptr)
	{
		// TODO ? лучше не вызывать напрямую здесь команду, а "поднимать" событие о том, что появился запрос на перемещение

		SketchMusic::PositionedSymbol^ psym = dynamic_cast<SketchMusic::PositionedSymbol^>(_dragged->DataContext);
		if (psym)
		{
			auto newPos = GetPositionOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);
			if (!(psym->_pos->EQ(newPos)))
			{
				_CommandManager->AddAndExecute(
					MoveSymCommand,
					ref new SketchMusic::Commands::SymbolPairHandlerArgs(
						dynamic_cast<SketchMusic::Text^>(_dragged->Tag),
						psym,
						ref new PositionedSymbol(
							newPos,
							psym->_sym)));
			}
			else
			{
				// если вычисленная новая позиция та же, просто отрисовываем
				SetNoteOnCanvas(_dragged);
			}
		}
	}

	_isMoving = false;
	_dragged->Foreground = nullptr;
	//SetBackgroundColor(_dragged);
	_dragged = nullptr;
	_scrollViewer->HorizontalScrollMode = ScrollMode::Enabled;
	_scrollViewer->VerticalScrollMode = ScrollMode::Enabled;

	//VisualStateManager::GoToState(bt, "Normal", true);
}

// смещаем курсор на единицу влево
void SketchMusic::View::TextRow::MoveCursorLeft()
{
	// сдвигать должны на одну позицию согласно масштабированию
	if (currentPosition->LE(_startPos))
		return;

	int beat = currentPosition->Beat;
	float tick = currentPosition->Tick;
	
	// округялем с точностью до квантизации
	int roundedTicks = round(tick * quantize);
	//beat -= 1 / this->initialised;
	roundedTicks -= TICK_IN_BEAT; //  /quantize;
	tick = (float)roundedTicks / quantize;
	
	if (tick < 0)
	{
		tick += TICK_IN_BEAT;
		beat -= 1;
	}

	if (tick < 0)
		tick = 0;
	if (beat < _startPos->Beat)
	{
		beat = _startPos->Beat;
		tick = 0;
	}
	SetCursor(ref new Cursor(beat, tick));
}

void SketchMusic::View::TextRow::MoveCursorRight()
{
	// сдвигать должны на одну позицию согласно масштабированию
	if (!(currentPosition->LT(_maxPos)))	// за неимением GE=GreaterOrEqual
		return;

	int beat = currentPosition->Beat;
	float tick = currentPosition->Tick;
	
	// округялем с точностью до квантизации
	int roundedTicks = round(tick * quantize);
	//beat -= 1 / this->initialised;
	roundedTicks += TICK_IN_BEAT; //  /quantize;
	tick = (float)roundedTicks / quantize;

	if (tick >= TICK_IN_BEAT)
	{
		tick -= TICK_IN_BEAT;
		beat += 1;
	}
	if (beat >= _maxPos->Beat)
	{
		tick = 0;
		beat = _maxPos->Beat;
	}

	SetCursor(ref new Cursor(beat, tick));
}

// изменяем масштаб согласно переданному "фактору"
void SketchMusic::View::TextRow::SetScale(int scaleFactor)
{
	int newScale = (scaleFactor >= 0)
		? scale * scaleFactor
		: scale / abs(scaleFactor);

	if (newScale > 16)
		newScale = 16;
	if (newScale < 1)
		newScale = 1;

	if (newScale != scale)
	{
		// изменяем ширину долей
		for (auto&& row : _mainPanel->Children)
		{
			auto rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(row));
			if (rowPanel == nullptr)
				continue;

			for (auto&& child : rowPanel->Children)
			{
				auto beat = dynamic_cast<ContentControl^>(static_cast<Object^>(child));
				if (beat == nullptr)
					continue;

				beat->Width += (newScale > scale) ? PlaceholderWidth : -PlaceholderWidth;
			}
		}

		_mainPanel->UpdateLayout();

		scale = newScale;
		//quantize = newScale;

		// если курсор приходится на дробную позицию согласно новому масштабу, округляем
		int roundedTicks = (int)round(currentPosition->Tick * quantize);
		if (roundedTicks % TICK_IN_BEAT)
			currentPosition->setPos(currentPosition->Beat, currentPosition->Tick * quantize / TICK_IN_BEAT);

		// обновляем положения нот
		RedrawText();
	}
}
