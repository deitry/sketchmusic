//
// TextRow.cpp
// Реализация класса TextRow.
//

#include "pch.h"
#include "TextRow.h"
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

// Документацию по шаблону элемента "Элемент управления на основе шаблона" см. по адресу http://go.microsoft.com/fwlink/?LinkId=234235

SketchMusic::View::TextRow::TextRow()
{
	// инициализируем поля, которые не зависят от шаблона
	DefaultStyleKey = "SketchMusic.View.TextRow";

	initialised = 0;
	scale = 1;

	currentPosition = ref new Cursor;

	// связываем с обработчиками
	//this->PointerWheelChanged += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerWheelChanged);
	this->Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::TextRow::OnLoaded);
}

/**
Создаём столько элементов привязки, сколько нужно
Может вообще отдельные объекты создавать не надо, а можно обойтись по одному объекту на долю?
При масштабировании увеличивать ширину объекта
А точки привязки будут очень условные - 
- по обработчику движения (лучше PointerEntered?) мыши определяем текущий контрол - текущую долю
- исходя из scale решаем сколько должно быть точек привязки и рисуем один-единственный кружок,
соответствующий ближайшей к указателю точке
*/
void SketchMusic::View::TextRow::AllocateSnapPoints(SketchMusic::Text^ text, int newScale)
{
	if (text == nullptr)
		return;

	//if (initialised >= newScale)
	//	return;

	// что будет делать функция в новой редакции
	// - проходимся по тексту
	// - составляем список: номер строки - положение крайнего символа 
	// (разрыв строки или условная длина (32 доли, например) для последней строки)
	// - создаём столько строк, сколько должно быть
	// - - для каждой строки создаём столько долей, сколько должно быть
	// - - если разрыв строки посреди доли, то одна доля входит дважды - на конце одной строки и в начале следующей.
	// Она должна по-особому обрабатываться в случае поиска контрола и для отрисовки-обработки движения указателя-поиска точки привязки 

	// очищаем
	// TODO : не удалять, добавлять к уже имеющемуся
	breakLine.clear();
	_mainPanel->Children->Clear();
	// TODO : проверка на то, что у количество строк и количество разделов строк совпадают

	// TODO : проходимся по тексту и выясняем, когда у него имеются разрывы строк
	//int maxBeat = 0;
	//for (auto&& sym : text->getText())
	//{
	//	maxBeat = sym->_pos->getBeat();
		// TODO : вставлять разрывы строк принудительно, если продолжительность
		// данной строки превышает N долей
	//	if (dynamic_cast<SketchMusic::SNewLine^>(static_cast<Object^>(sym->_sym)))
	//	{
	//		breakLine.push_back(sym);	// ref new PositionedSymbol - чтобы не ссылаться на тот же самый элемент
	//	}
	//}

	// + ещё одна строка - последняя
	breakLine.push_back(ref new PositionedSymbol(ref new Cursor(32),ref new SNewLine));	

	int prev = 0;

	for (int rowCnt = 0; rowCnt < breakLine.size(); rowCnt++)
	{
		int current = breakLine[rowCnt]->_pos->getBeat(); 
		// добавляем новую строку
		StackPanel^ row = ref new StackPanel;
		row->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("RowPanel"));
		_mainPanel->Children->Append(row);

		// заполняем долями
		for (int i = 0; i < (current - prev); i++)
		{
			ContentControl^ ctrl = ref new ContentControl;
			ctrl->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
			ctrl->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
			ctrl->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("PlaceholderControlStyle"));

			row->Children->Append(ctrl);
		}

		prev = current;
	}

	this->_maxPos = ref new Cursor(prev - 1);

	_mainPanel->UpdateLayout();
	scale = newScale;
}

void SketchMusic::View::TextRow::InsertLineBreak(Cursor^ pos)
{
	// вставка разрыва строки
	// - находим строку, в которую будем вставлять разрыв
	int beat = pos->getBeat();
	int ctrlCnt = 0;

	for (int i = 0; i < _mainPanel->Children->Size; i++)
	{
		auto rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(_mainPanel->Children->GetAt(i)));
		if (rowPanel == nullptr)
			continue;

		ctrlCnt += rowPanel->Children->Size;
		if (beat < ctrlCnt)
		{
			// - вычисляем индекс нового крайнего элемента
			int rowEnd = ctrlCnt - beat;

			// - создаём новую строку, вставляем в _mainPanel
			StackPanel^ newRow = ref new StackPanel;
			newRow->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("RowPanel"));
			_mainPanel->Children->InsertAt(i, newRow);

			// - аналогично добавляем значение в массив "крайних точек"
			auto psym = ref new PositionedSymbol(ref new Cursor(beat), ref new SNewLine);
			breakLine.insert(breakLine.begin() + i, psym);
			//current->addSymbol(psym);

			// - переносим в новую строку все контролы после нового крайнего
			while (rowEnd < rowPanel->Children->Size)
			{
				auto element = rowPanel->Children->GetAt(rowEnd);
				rowPanel->Children->RemoveAt(rowEnd);
				newRow->Children->Append(element);
			}

			// - если это была последняя строка, то заполняем её до маскимума

			break;
		}
	}

	// - перерисовываем текст
	_mainPanel->UpdateLayout();
	RedrawText();
}

void SketchMusic::View::TextRow::DeleteLineBreak(Cursor^ pos)
{
	// удаление разрыва строки
	// - находим строку, из которой будем удалять разрыв
	int beat = pos->getBeat();
	int rowCnt = 0; // конец нулевой строки - начало первой

	for (auto&& line : breakLine)
	{
		if (line->_pos->EQ(pos))
		{
			// та строка которую нужно удалить
			auto first = dynamic_cast<StackPanel^>(_mainPanel->Children->GetAt(rowCnt));
			auto second = dynamic_cast<StackPanel^>(_mainPanel->Children->GetAt(rowCnt + 1));

			if ((first == nullptr) || (second == nullptr))
			{
				return;
			}

			// - переносим в предыдущую строку все элементы, если это не превысит лимит (?)
			while (second->Children->Size > 0)
			{
				// TODO : можно сделать поэффективнее
				auto child = second->Children->GetAt(0);
				second->Children->RemoveAt(0);
				first->Children->Append(child);
			}

			// - если это была последняя строка, то обрезаем её до маскимума
			// - вычисляем индекс нового крайнего элемента, удаляем элемент из вектора, 
			// в котором хранятся разрывы строк, обновляем значение предыдущего
			
			// - удалем текущую строку
			_mainPanel->Children->RemoveAt(rowCnt + 1);
			breakLine[rowCnt]->_pos->moveTo(breakLine[rowCnt + 1]->_pos);
			breakLine.erase(breakLine.begin() + rowCnt + 1);
			break;
		}
		rowCnt++;
	}
	
	// - перерисовываем текст
	_mainPanel->UpdateLayout();
	RedrawText();
}

void SketchMusic::View::TextRow::SetText(SketchMusic::Text^ text) 
{
	// добавляем текст в список
	// делаем его "главным"
	unsigned int ndx = 0;
	if (!data->texts->IndexOf(text, &ndx))
		this->data->texts->Append(text);

	current = text;

	if (_mainPanel == nullptr)
		return;

	AllocateSnapPoints(text, 1);
	InvalidateText();
	RedrawText();
}

void SketchMusic::View::TextRow::SetText(SketchMusic::CompositionData^ textCollection, SketchMusic::Text^ format)
{
	if (data == textCollection)
		return;

	data = textCollection;
	
	if (format)
	{
		this->format = format;
		this->current = format;
	}
	else
	{
		this->format = textCollection->texts->First()->Current;
		this->current = this->format;
	}

	InvalidateText();
}

/*
Удаляем объекты нот и создаём их заново.
*/
void SketchMusic::View::TextRow::InvalidateText()
{
	// отрисовываем объекты согласно тексту
	// чтобы не делать лишнюю работу, нужна функция, которая сразу принимала бы
	// список текстов для отрисовки и текст, который нужно взять за основу форматирования
	if (this->data->texts->Size == 0)
		return;

	// приводим набор нот в соответствие с текстом
	if (_canvas == nullptr)
		return;

	_canvas->Children->Clear();
	_canvas->Children->Append(_snapPoint);
	_canvas->Children->Append(_cursor);

	// TODO : add для символов форматирования - только для текста, чьё форматирование сейчас берётся за основу
	//for (auto text : _texts)
	//{
		auto symbols = current->getText();
		for (auto&& symbol : symbols)
		{
			// - проверка на то, что если текст отличен от текста форматирования и это не нота - пропускаем
			
			this->AddSymbol(symbol);
		}
	//}
	// перерисовываем
	//RedrawText();
}

/*
Перерисовываем уже нарисованные ноты
*/
void SketchMusic::View::TextRow::RedrawText()
{
	// перерисовываем ноты
	// чтобы не создавать новые объекты
	for (auto note : notes)
	{
		SetNoteOnCanvas(note);
	}

	// перерисовываем положение курсора
	auto point = GetCoordinatsOfPosition(currentPosition);
	_canvas->SetLeft(_cursor, point.X);
	_canvas->SetTop(_cursor, point.Y);
}

SketchMusic::Text^ SketchMusic::View::TextRow::GetText()
{ 
	// возвращаем "главный" текст
	return current;
}

// работа с отдельными символами
void SketchMusic::View::TextRow::AddSymbol(PositionedSymbol^ sym)
{
	
	// отыскиваем, куда нужно ставить
	// вставляем
	if (dynamic_cast<SketchMusic::SSpace^>(sym->_sym))
	{
		auto ctrl = GetControlAtPos(sym->_pos, -1);	// получаем предыдущий ктрл
		if (ctrl == nullptr)
			return;

		ctrl->Width += BeatWidth;
		_mainPanel->UpdateLayout();
		return;
	}

	if (dynamic_cast<SketchMusic::SNewLine^>(sym->_sym))
	{
		// вставляем разрыв строки
		this->InsertLineBreak(sym->_pos);
		return;
	}

	// создаём объект символа
	Windows::UI::Xaml::Controls::ContentControl^ bt = ref new Windows::UI::Xaml::Controls::ContentControl;
	bt->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("SymbolControlStyle"));
	bt->Content = sym;
	bt->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
	bt->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
	bt->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);
	bt->Tag = current;
	// изменяем цвет согласно тегу
	SetBackgroundColor(bt);

	// привязываем свежесозданный символ к канвасу, отвечающему за соответствующую долю
	_canvas->Children->Append(bt);
	notes.push_back(bt);

	SetNoteOnCanvas(bt);
}

// скрыть ненужные точки привязки
// показать нужные
void SketchMusic::View::TextRow::SetSnapPointsVisibility(int newScale)
{
	// проходимся по всем плейсхолдерам
	for (auto row : _mainPanel->Children)
	{
		auto rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(row));
		int i = 0;
		for (auto&& phold : rowPanel->Children)
		{
			// степень вложенности привязок определяем по тегам ... ну и по индексу
			int tick = i*newScale % initialised;
			if (tick == 0)
			{
				phold->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			else
			{
				phold->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			}
			i++;
		}
	}
	_mainPanel->UpdateLayout();
}

void SketchMusic::View::TextRow::InsertNoteObject(PositionedSymbol^ note)
{
	/*
	// обрабатываем символы форматирования
	if (dynamic_cast<SketchMusic::SSpace^>(sym->_sym))
	{
		Windows::UI::Xaml::Controls::Canvas^ cnv = _placeholders->GetAt(sym->_pos->getBeat() - 1);	// берём предыдущий плейсхолдер
		cnv->Width += (double)(this->Resources->Lookup("PlaceholderWidth"));
		return;
	}

	// создаём объект символа
	Windows::UI::Xaml::Controls::ContentControl^ bt = ref new Windows::UI::Xaml::Controls::ContentControl;
	bt->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(this->Resources->Lookup("SymbolButtonStyle"));
	bt->Content = sym;
	bt->Holding += ref new Windows::UI::Xaml::Input::HoldingEventHandler(this, &StrokeEditor::MainPage::OnHolding);
	bt->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &StrokeEditor::MainPage::onSymbolPointerPressed);
	bt->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &StrokeEditor::MainPage::_panel_PointerReleased);
	bt->Tag = "note";

	// привязываем свежесозданный символ к канвасу, отвечающему за соответствующую долю
	Windows::UI::Xaml::Controls::Canvas^ cnv = _placeholders->GetAt(sym->_pos->getBeat());
	cnv->Children->Append(bt);
	cnv->SetLeft(bt, sym->_pos->getTick()*cnv->Width / 32);
	cnv->SetTop(bt, 0);
	*/
}

int SketchMusic::View::TextRow::GetControlIndexAtPos(Cursor^ pos, int offset)
{
	int lookupIndex = pos->getBeat()*initialised + pos->getTick() * scale / TICK_IN_BEAT * initialised / scale;
	lookupIndex += offset;
	return lookupIndex;
}

// по факту нам сам контрол редко бывает нужен, нужнее точка его левого верхнего угла
// поэтому стоит возвращать Point
ContentControl^ SketchMusic::View::TextRow::GetControlAtPos(Cursor^ pos, int offset)
{
	int currentIndex = 0;
	int lookupIndex = GetControlIndexAtPos(pos, offset);

	if (lookupIndex < 0)
		return nullptr;

	if (_mainPanel == nullptr)
		return nullptr;

	for (auto&& row : _mainPanel->Children)
	//auto iter = _mainPanel->Children->First();
	//while (iter->HasCurrent)
	{
		StackPanel^ rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(row));
		int max = rowPanel->Children->Size + currentIndex;
		if (lookupIndex < max)
		{
			ContentControl^ ctrl = dynamic_cast<ContentControl^>(rowPanel->Children->GetAt(lookupIndex - currentIndex));
			return ctrl;
		}

		currentIndex += rowPanel->Children->Size;
		//iter->MoveNext();
	}

	return nullptr;
}

float SketchMusic::View::TextRow::CalculateTick(float offsetX, ContentControl ^ ctrl)
{
	return ((int)((offsetX + ctrl->Width/scale / 4) / ctrl->Width * scale)) * TICK_IN_BEAT / scale;
	//int tick = (int)(offset.X / ctrl->Width * TICK_IN_BEAT * scale / TICK_IN_BEAT) * TICK_IN_BEAT / scale;
}

// функцию стоит изменить, т.к. храня только доли мы не сможем отличить одно положение от другого только по контролу
// Следует передавать текущее вычисленное значение точки привязки. Может быть, стоит пересылать связку Ctrl-sender + Point относительно Ctrl'а,
// этого должно быть достаточно, чтобы точно вычислить значение курсора, соответствующее текущей точке
// Cursor GetPositionAtPoint(ContentControl, Point)
Cursor^ SketchMusic::View::TextRow::GetPositionOfControl(Windows::UI::Xaml::Controls::ContentControl^ ctrl, Point offset)
{
	if ((_mainPanel == nullptr) || (ctrl == nullptr))
		return nullptr;

	// - проходимся по строкам
	// - вычисляем индекс=долю текущего контрола
	// - с учётом текущего масштаба по положению курсора относительно левого верхнего угла 
	// вычисляем "дробный" индекс - смещение в тиках

	int currentRowIndex = 0;
	for (auto&& row : _mainPanel->Children)
	{
		StackPanel^ rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(row));
		unsigned int index = 0;
		if (rowPanel->Children->IndexOf(ctrl, &index))
		{
			float tick = CalculateTick(offset.X, ctrl);
			Cursor^ position = ref new Cursor(index + currentRowIndex, tick);
			return position;
		}

		currentRowIndex += rowPanel->Children->Size;
	}

	return nullptr;
}

Point SketchMusic::View::TextRow::GetCoordinatsOfPosition(Cursor^ pos)
{
	ContentControl^ ctrl = this->GetControlAtPos(pos);
	auto transform = ctrl->TransformToVisual(_canvas);
	Point basePoint = Point(0, 0);
	auto point = transform->TransformPoint(basePoint);

	// смещение учитывая тики
	point.X += ctrl->Width * pos->getTick() / TICK_IN_BEAT;

	return point;
}

// вспомогательная функция для тех случаев, когда у нас уже есть контрол и нам нужно только округлить x
Point SketchMusic::View::TextRow::GetCoordinatsOfControl(Windows::UI::Xaml::Controls::ContentControl^ ctrl, Point offset)
{
	if ((_mainPanel == nullptr) || (ctrl == nullptr))
		return Point(0,0);
	
	auto transform = _currentSnapPoint->TransformToVisual(_canvas);
	Point basePoint = Point(0, 0);
	auto point = transform->TransformPoint(basePoint);

	// прибавка к x чтобы можно было "кидать" на точку привязки не только справа, но и чуть-чуть слева
	float tick = CalculateTick(offset.X, ctrl);
	point.X += tick * ctrl->Width / TICK_IN_BEAT;

	return point;
}

void SketchMusic::View::TextRow::SetBackgroundColor(ContentControl^ ctrl)
{
	auto psym = reinterpret_cast<SketchMusic::PositionedSymbol^>(ctrl->Content);
	switch (psym->_sym->GetSymType())
	{
	case SketchMusic::SymbolType::NOTE:
	case SketchMusic::SymbolType::RNOTE:
	case SketchMusic::SymbolType::GNOTE:
		// не хочется делать проверки на каждый noteBackground. Считаем, что если один нашёлся, то и остальные будут
		if (_dict->HasKey("noteBackground"))
		{
			unsigned int ndx;
			if (data->texts->IndexOf(current, &ndx))
			{
				Windows::UI::Xaml::Media::Brush^ brush;
				switch (ndx)
				{
				case 1:
					brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("noteBackground2"));
					ctrl->Background = brush;
					break;
				case 2:
					brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("noteBackground3"));
					ctrl->Background = brush;
					break;
				case 3:
					brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("noteBackground4"));
					ctrl->Background = brush;
					break;
				case 0:
				default:
					brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("noteBackground"));
					ctrl->Background = brush;
					break;
				}
			}
		}
		break;
	case SketchMusic::SymbolType::TEMPO:
		if (_dict->HasKey("tempoBackground"))
		{
			auto brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("tempoBackground"));
			ctrl->Background = brush;
		}
		break;
	}

	
}

double SketchMusic::View::TextRow::GetOffsetY(ISymbol ^ sym)
{
	double offsetY = 0;
	// если обрабатываемый символ - нота
	INote^ inote = dynamic_cast<INote^>(sym);
	if (inote)
	{
		// TODO : сделать расчёт похитрее, чтобы выглядело как на нотном стане
		// В том числе разрещить вылезать за пределы, увеличивая тем самым высоту строки
		// (чтобы строки не налезали друг на друга)
		offsetY = -abs(((inote->_val) % 12));
		offsetY *= RowHeight / 12;
		offsetY += RowHeight - 5;
		//if (offsetY > 6) offsetY -= 12;
		//if (offsetY < -6) offsetY += 12;
		
	} else {
		STempo^ tempo = dynamic_cast<STempo^>(sym);
		if (tempo)
		{
			offsetY = -0.6*RowHeight;
		}
	}
	return offsetY;
}

void SketchMusic::View::TextRow::SetNoteOnCanvas(ContentControl ^ note)
{
	PositionedSymbol^ psym = dynamic_cast<PositionedSymbol^>(note->Content);
	if (psym == nullptr) return;

	auto point = GetCoordinatsOfPosition(psym->_pos);
	_canvas->SetLeft(note, point.X - PlaceholderWidth / 2);
	double offsetY = GetOffsetY(psym->_sym);
	_canvas->SetTop(note, point.Y + offsetY);
}
