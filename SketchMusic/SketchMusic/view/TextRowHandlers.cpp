#include "pch.h"
#include "TextRow.xaml.h"
#include "../base/Text.h"

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

void SketchMusic::View::TextRow::OnApplyTemplate()
{
	
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

	//_canvas = (Canvas^)GetTemplateChild("_canvas");
	_canvas->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
	_canvas->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);

	_snapPoint = ref new ContentControl;
	_snapPoint->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
	_snapPoint->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("SnapPointStyle"));
	_snapPoint->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	_canvas->Children->Append(_snapPoint);

	//_cursor = (ContentControl^)GetTemplateChild("_cursor");

	//_scrollViewer = (ScrollViewer^)GetTemplateChild("_scrollViewer");

	if (this->data->texts->Size > 0)
	{
		AllocateSnapPoints(this->GetText(), 1);
	}

	initialised = 1;
	scale = 1;
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

// выбор ноты или чего-нибудь ещё
void SketchMusic::View::TextRow::OnPointerPressed(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
	// если второй палец 
	// - если нота уже "поднята", ничего не происходит
	// - если нота не "поднята", то замеряем расстояние между пальцами
	// - - если оно меняется, то при достижении определённой "дельты" вызываем SetScale
	
	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if ((ctrl == nullptr) || (dynamic_cast<Text^>(ctrl->Tag) != current))
	{
		// если нажато один раз, но ноты нет, зато есть контрол, то перемещаем указатель в данный контрол
		if (_currentSnapPoint)
		{
			Cursor^ newPos = GetPositionOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);
			if (newPos)
			{
				currentPosition->moveTo(newPos);

				auto point = GetCoordinatsOfPosition(newPos);
				_canvas->SetLeft(_cursor, point.X);
				_canvas->SetTop(_cursor, point.Y);
			}
		}
		return;
	}

	//_scrollViewer->HorizontalScrollMode = ScrollMode::Disabled;
	ScrollViewer::SetHorizontalScrollMode(_mainPanel, ScrollMode::Disabled);
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
	auto curPoint = e->GetCurrentPoint(nullptr)->Position;
	auto listItems = VisualTreeHelper::FindElementsInHostCoordinates(curPoint, _mainPanel);
	
	// - выделяем долю, которая под курсором
	for (auto item : listItems)
	{
		Windows::UI::Xaml::Controls::ContentControl^ ctrl = dynamic_cast<Windows::UI::Xaml::Controls::ContentControl^>(item);
		if ((ctrl == nullptr) || (((String^)ctrl->Tag) != "phold"))
			continue;

		if (ctrl != _currentSnapPoint)
		{
			//if (_currentSnapPoint)
			//{
			//	_currentSnapPoint->Background = ref new SolidColorBrush(Windows::UI::Colors::Transparent);
			//}
			//ctrl->Background = ref new SolidColorBrush(Windows::UI::Colors::Orange);
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
	if (_dragged == nullptr)
		return;

	if (_currentSnapPoint != nullptr)
	{
		SketchMusic::PositionedSymbol^ psym = dynamic_cast<SketchMusic::PositionedSymbol^>(_dragged->Content);
		//INote^ inote = dynamic_cast<INote^>(psym->_sym);
		// привязываем к ближайшей точке привязки
		auto point = GetCoordinatsOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);
		_canvas->SetLeft(_dragged, point.X - PlaceholderWidth/2);
		double offsetY = GetOffsetY(psym->_sym);
		_canvas->SetTop(_dragged, point.Y + offsetY);

		// перемещаем символ "фактически"
		Text^ text = current;
		if (psym && text)
		{
			SketchMusic::Cursor^ newpos = this->GetPositionOfControl(_currentSnapPoint, e->GetCurrentPoint(_currentSnapPoint)->Position);

			// вообще, если мы до сюда дошли, то всё должно быть хорошо
			if (newpos == nullptr)
				throw ref new Exception(1, "Не удалось вычислить новое положение ноты");

			text->moveSymbol(psym, newpos);
			psym->_pos = newpos;

			// TODO : вызываем событие, которое говорит, что такая-то нота была перемещена
			// без события мы не сможем записать в основной программе в CommandManager команду,
			// а значит и не сможем в будущем отменить перемещение
		}
	}

	_isMoving = false;
	_dragged->Foreground = nullptr;
	//SetBackgroundColor(_dragged);
	_dragged = nullptr;
	ScrollViewer::SetHorizontalScrollMode(_mainPanel, ScrollMode::Enabled);
	//_scrollViewer->HorizontalScrollMode = ScrollMode::Enabled;

	//VisualStateManager::GoToState(bt, "Normal", true);
}

// смещаем курсор на единицу влево
void SketchMusic::View::TextRow::MoveCursorLeft()
{
	// сдвигать должны на одну позицию согласно масштабированию
	if ((currentPosition->getBeat() == 0) && (currentPosition->getTick() == 0))
		return;

	int beat = currentPosition->getBeat();
	int tick = currentPosition->getTick();
	//beat -= 1 / this->initialised;
	tick -= TICK_IN_BEAT / scale;	// не очень хорошо пользоваться, пока функции не отлажены
	if (tick < 0)
	{
		tick += TICK_IN_BEAT;
		beat -= 1;
	}

	if (tick < 0)
		tick = 0;
	if (beat < 0)
	{
		beat = 0;
		tick = 0;
	}
	currentPosition->moveTo(ref new Cursor(beat, tick));

	auto point = this->GetCoordinatsOfPosition(currentPosition);
	_canvas->SetLeft(_cursor, point.X);
	_canvas->SetTop(_cursor, point.Y);
}

void SketchMusic::View::TextRow::MoveCursorRight()
{
	// сдвигать должны на одну позицию согласно масштабированию
	if (currentPosition->EQ(_maxPos))	// TODO : получать максимально возможное значение от this 
		return;

	int beat = currentPosition->getBeat();
	int tick = currentPosition->getTick();
	tick += TICK_IN_BEAT / scale;
	if (tick >= TICK_IN_BEAT)
	{
		tick -= TICK_IN_BEAT;
		beat += 1;
	}

	currentPosition->moveTo(ref new Cursor(beat, tick));

	auto point = this->GetCoordinatsOfPosition(currentPosition);
	_canvas->SetLeft(_cursor, point.X);
	_canvas->SetTop(_cursor, point.Y);
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

		// если курсор приходится на дробную позицию согласно новому масштабу, округляем
		if (currentPosition->getTick() * scale % TICK_IN_BEAT)
			currentPosition->setPos(currentPosition->getBeat(), currentPosition->getTick() * scale / TICK_IN_BEAT);

		// обновляем положения нот
		RedrawText();
	}
}

// удаляем все ноты, находящиеся между текущим и положением курсора на единицу влево согласно текущему масштабу
void SketchMusic::View::TextRow::Backspace()
{	
	// если курсор стоит в начале новой строки, то удаляем строку, символы переносим в строку предыдущую
	for (auto&& iter : breakLine)
	{
		if (currentPosition->EQ(iter->_pos))
		{
			this->DeleteLineBreak(currentPosition);
			this->current->deleteNLine(currentPosition);
			return;
		}
	}

	// берём положение курсора
	// убеждаемся, что оно ненулевое
	if ((currentPosition->getBeat() == 0) && (currentPosition->getTick() == 0))
		return;

	// вычисляем положение курсора на единицу влево
	int beat = currentPosition->getBeat();
	int tick = currentPosition->getTick();
	tick -= TICK_IN_BEAT / scale;
	if (tick < 0)
	{
		tick += TICK_IN_BEAT;
		beat -= 1;
	}

	Cursor^ to = ref new Cursor(beat, tick);

	// удаляем символы из текста
	current->deleteSymbols(currentPosition, to);

	// удаляем все элементы с текущего положения по положение на шаг назад
	int i = 0;
	while (i < _canvas->Children->Size)
	{
		auto ctrl = dynamic_cast<ContentControl^>(_canvas->Children->GetAt(i));
		if (ctrl)
		{
			// по тегу проверяем, что нота принадлежит текущей дорожке
			auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(ctrl->Content);
			if (((Text^)ctrl->Tag == current) && (psym != nullptr))
			{
				// если она попадает в искомый диапазон, то удаляем
				if (psym->_pos->LT(currentPosition)
					&& ((psym->_pos->GT(to)) || (psym->_pos->EQ(to))))
				{
					_canvas->Children->RemoveAt(i);
					continue;
				}
			}
		}
		i++;
	}

	// перемещаем курсор на шаг назад
	this->MoveCursorLeft();
}