//
// TextRow.cpp
// Реализация класса TextRow.
//

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

// Документацию по шаблону элемента "Элемент управления на основе шаблона" см. по адресу http://go.microsoft.com/fwlink/?LinkId=234235

SketchMusic::View::TextRow::TextRow()
{
	InitializeComponent();

	// инициализируем поля, которые не зависят от шаблона
	//DefaultStyleKey = "SketchMusic.View.TextRow";

	initialised = 0;
	scale = 2;
	quantize = 4;

	currentPosition = ref new Cursor;
	data = ref new CompositionData;

	InitializePage();

	// связываем с обработчиками
	//this->PointerWheelChanged += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerWheelChanged);
	this->Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &SketchMusic::View::TextRow::OnLoaded);
}

/**
Создаём столько элементов привязки, сколько нужно
При масштабировании увеличивать ширину объекта
А точки привязки будут очень условные -
- по обработчику движения (лучше PointerEntered?) мыши определяем текущий контрол - текущую долю
- исходя из quantize решаем сколько должно быть точек привязки и рисуем один-единственный кружок,
соответствующий ближайшей к указателю точке
*/
void SketchMusic::View::TextRow::AllocateSnapPoints(SketchMusic::Text^ text, int newScale, int selectedPart)
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
	// находим нужную часть и определяем
	if (selectedPart >= 0)	// по умолчанию если нам это не надо, selectedPart = -1
	{
		int cnt = 0;
		for (auto&& iter : data->ControlText->_t)
		{
			if (iter.second->GetSymType() == SymbolType::NPART)
			{
				if (cnt == selectedPart)
				{
					// искомая часть
					_startPos = ref new Cursor(iter.first);
				}
				else if (cnt == selectedPart + 1)
				{
					// конец искомой части
					_maxPos = ref new Cursor(iter.first);
					break;
				}
				cnt++;
			}
		}
	}
	if (_startPos == nullptr) _startPos = ref new Cursor;
	if (_maxPos == nullptr) _maxPos = ref new Cursor(64);
	breakLine.push_back(ref new PositionedSymbol(ref new Cursor(_maxPos), ref new SNewLine));
	
	int prev = _startPos->Beat;

	for (int rowCnt = 0; rowCnt < breakLine.size(); rowCnt++)
	{
		int current = breakLine[rowCnt]->_pos->Beat;
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
			ctrl->RightTapped += ref new Windows::UI::Xaml::Input::RightTappedEventHandler(this, &SketchMusic::View::TextRow::OnRightTapped);
			ctrl->Holding += ref new Windows::UI::Xaml::Input::HoldingEventHandler(this, &SketchMusic::View::TextRow::OnHolding);
			ctrl->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("PlaceholderControlStyle"));
			ctrl->DataContext = (prev + i + 1);
			// TODO : настраивать Width согласно newScale
			if (newScale >= 2)
				ctrl->Width += PlaceholderWidth;	// пока только для масштаба 2 сделаем
			row->Children->Append(ctrl);
		}
		
		// меняем стиль последнего пхолдера, чтобы обозначить конец строки
		ContentControl^ ctrlEnd = dynamic_cast<ContentControl^>(row->Children->GetAt(row->Children->Size - 1));
		if (ctrlEnd)
		{
			ctrlEnd->BorderThickness = Thickness(1,0,1,0);
		}

		prev = current;
	}

	_mainPanel->UpdateLayout();
	scale = newScale;
	//quantize = scale;
}

void SketchMusic::View::TextRow::InsertLineBreak(Cursor^ pos)
{
	// вставка разрыва строки
	// - находим строку, в которую будем вставлять разрыв
	int difBeat = pos->Beat - _startPos->Beat;
	int ctrlCnt = 0;

	for (int i = 0; i < _mainPanel->Children->Size; i++)
	{
		auto rowPanel = dynamic_cast<StackPanel^>(static_cast<Object^>(_mainPanel->Children->GetAt(i)));
		if (rowPanel == nullptr)
			continue;

		ctrlCnt += rowPanel->Children->Size;
		if (difBeat < ctrlCnt)
		{
			// - вычисляем индекс нового крайнего элемента
			int rowEnd = ctrlCnt - difBeat;

			// - создаём новую строку, вставляем в _mainPanel
			StackPanel^ newRow = ref new StackPanel;
			newRow->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("RowPanel"));
			_mainPanel->Children->InsertAt(i, newRow); // новая строка встаёт ПЕРЕД старой

			// - аналогично добавляем значение в массив "крайних точек"
			auto psym = ref new PositionedSymbol(ref new Cursor(pos->Beat), ref new SNewLine);
			breakLine.insert(breakLine.begin() + i, psym);
			//current->addSymbol(psym);

			// - переносим в новую строку все контролы после нового крайнего
			while (rowEnd < rowPanel->Children->Size)
			{
				auto element = rowPanel->Children->GetAt(0);
				rowPanel->Children->RemoveAt(0);
				newRow->Children->Append(element);
			}
			
			// меняем стиль последнего пхолдера, чтобы обозначить конец строки
			ContentControl^ ctrlEnd = dynamic_cast<ContentControl^>(newRow->Children->GetAt(newRow->Children->Size - 1));
			if (ctrlEnd)
			{
				ctrlEnd->BorderThickness = Thickness(1, 0, 1, 0);
			}

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
	int beat = pos->Beat - _startPos->Beat;
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

			// меняем стиль последнего пхолдера, чтобы обозначить конец строки
			ContentControl^ ctrlEnd = dynamic_cast<ContentControl^>(first->Children->GetAt(first->Children->Size - 1));
			if (ctrlEnd)
			{
				ctrlEnd->BorderThickness = Thickness(1, 0, 0, 0);
			}

			// - переносим в предыдущую строку все элементы
			while (second->Children->Size > 0)
			{
				// TODO : можно сделать поэффективнее?
				auto child = second->Children->GetAt(0);
				second->Children->RemoveAt(0);
				first->Children->Append(child);
			}

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

void SketchMusic::View::TextRow::SetText(SketchMusic::CompositionData^ textCollection, SketchMusic::Text^ selected)
{
	SetText(textCollection, selected, -1);
}

void SketchMusic::View::TextRow::SetText(CompositionData ^ textCollection, SketchMusic::Text ^ selected, int selectedPart)
{
	if (data != textCollection)
		data = textCollection;

	if (textCollection->texts && textCollection->texts->Size > 0)
	{
		if (selected)
		{
			this->format = selected;
			this->current = selected;
		}
		else
		{
			this->format = textCollection->texts->First()->Current;
			this->current = this->format;
		}

		AllocateSnapPoints(this->format, 2, selectedPart);
		InvalidateText();
	}

	SetCursor(_startPos);
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
	
	auto symbols = current->GetSymbols(_startPos, _maxPos);
	for (auto&& symbol : symbols)
	{
		// - проверка на то, что если текст отличен от текста форматирования и это не нота - пропускаем

		this->AddSymbolView(current, symbol);
	}
	// аналогично - для управляющих символов
	symbols = data->ControlText->getText();
	for (auto&& symbol : symbols)
	{
		// - проверка на то, что если текст отличен от текста форматирования и это не нота - пропускаем

		this->AddSymbolView(data->ControlText, symbol);
	}
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
	SetCursor(currentPosition);
}

SketchMusic::Text^ SketchMusic::View::TextRow::GetText()
{
	// возвращаем "главный" текст
	return current;
}

void SketchMusic::View::TextRow::SetCursor(SketchMusic::Cursor^ pos)
{
	if (pos)
	{
		if (pos->LT(_startPos)) pos = _startPos;
		if (!pos->LE(_maxPos)) pos = _maxPos;

		currentPosition->moveTo(pos);
	}

	// перерисовываем положение курсора
	auto point = GetCoordinatsOfPosition(currentPosition);
	_canvas->SetLeft(_cursor, point.X);
	_canvas->SetTop(_cursor, point.Y);
}

// работа с отдельными символами
void SketchMusic::View::TextRow::AddSymbolView(Text^ source, PositionedSymbol^ sym)
{
	if (sym->_sym->GetSymType() == SymbolType::NPART) return;

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
	switch (sym->_sym->GetSymType())
	{
	case SymbolType::GNOTE:
		bt->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("GenericNoteCtrlStyle"));
		break;
	default:
		bt->Style = reinterpret_cast<Windows::UI::Xaml::Style^>(_dict->Lookup("SymbolControlStyle"));
		break;
	}
	bt->DataContext = sym;
	bt->PointerPressed += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerPressed);
	bt->PointerMoved += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerMoved);
	bt->PointerReleased += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &SketchMusic::View::TextRow::OnPointerReleased);
	bt->Tag = source;
	// изменяем цвет согласно тегу
	SetBackgroundColor(bt);

	// привязываем свежесозданный символ к канвасу, отвечающему за соответствующую долю
	_canvas->Children->Append(bt);
	notes.push_back(bt);

	SetNoteOnCanvas(bt);
}

// Удаление визуального представления символа
// TODO : убого как-то, надо переделать
void SketchMusic::View::TextRow::DeleteSymbolView(PositionedSymbol ^ symbol)
{
	if (dynamic_cast<SketchMusic::SNewLine^>(symbol->_sym))
	{
		// вставляем разрыв строки
		this->DeleteLineBreak(symbol->_pos);
		return;
	}

	int i = 0;
	while (i < _canvas->Children->Size)
	{
		auto ctrl = dynamic_cast<ContentControl^>(_canvas->Children->GetAt(i));
		if (ctrl)
		{
			// по тегу проверяем, что нота принадлежит текущей дорожке
			auto psym = dynamic_cast<SketchMusic::PositionedSymbol^>(ctrl->DataContext);
			if (((Text^)ctrl->Tag == current || (Text^)ctrl->Tag == data->ControlText) && (psym != nullptr))
			{
				// если это та самая нота
				if (psym->_pos->EQ(symbol->_pos) && (psym->_sym == symbol->_sym))
				{
					_canvas->Children->RemoveAt(i);
					continue;
				}
			}
		}
		i++;
	}
}

ContentControl ^ SketchMusic::View::TextRow::GetSymbolView(PositionedSymbol ^ symbol)
{
	for (auto&& child : _canvas->Children)
	{
		auto ctrl = dynamic_cast<ContentControl^>(static_cast<Object^>(child));
		if (ctrl == nullptr) continue;
		auto note = dynamic_cast<PositionedSymbol^>(ctrl->DataContext);
		if (note == nullptr) continue;

		//if ((note->_pos->EQ(symbol->_pos)) && (note->_sym->EQ(symbol->_sym))) return ctrl;
		if (symbol == note) return ctrl;
	}

	return nullptr;
}

int SketchMusic::View::TextRow::GetControlIndexAtPos(Cursor^ pos, int offset)
{
	//int lookupIndex = pos->getBeat()*initialised + pos->getTick() * scale / TICK_IN_BEAT * initialised / scale;
	int dif = pos->Beat - _startPos->Beat;
	int lookupIndex = dif*initialised + pos->Tick / TICK_IN_BEAT * initialised;
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
	return ((int)((offsetX + ctrl->Width / quantize / 4) / ctrl->Width * quantize)) * TICK_IN_BEAT / quantize;
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
			Cursor^ position = ref new Cursor(_startPos->Beat + index + currentRowIndex, tick);
			return position;
		}

		currentRowIndex += rowPanel->Children->Size;
	}

	return ref new Cursor;
}

Point SketchMusic::View::TextRow::GetCoordinatsOfPosition(Cursor^ pos)
{
	ContentControl^ ctrl;
	if (pos->EQ(_maxPos))
	{
		ctrl = this->GetControlAtPos(ref new Cursor(pos->Beat-1));
	}
	else
	{
		ctrl = this->GetControlAtPos(pos);
	}
	
	if (ctrl)
	{
		auto transform = ctrl->TransformToVisual(_canvas);
		Point basePoint = Point(0, 0);
		auto point = transform->TransformPoint(basePoint);

		// смещение учитывая тики
		point.X += ctrl->Width * pos->Tick / TICK_IN_BEAT;
		if (pos->EQ(_maxPos)) point.X += ctrl->Width;
		return point;
	}
	return Point(0,0);
}

// вспомогательная функция для тех случаев, когда у нас уже есть контрол и нам нужно только округлить x
Point SketchMusic::View::TextRow::GetCoordinatsOfControl(Windows::UI::Xaml::Controls::ContentControl^ ctrl, Point offset)
{
	if ((_mainPanel == nullptr) || (ctrl == nullptr))
		return Point(0, 0);

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
	auto psym = reinterpret_cast<SketchMusic::PositionedSymbol^>(ctrl->DataContext);
	switch (psym->_sym->GetSymType())
	{
	case SketchMusic::SymbolType::NOTE:
	case SketchMusic::SymbolType::RNOTE:
	case SketchMusic::SymbolType::GNOTE:
	case SketchMusic::SymbolType::END:
		// не хочется делать проверки на каждый noteBackground. Считаем, что если один нашёлся, то и остальные будут
		if (_dict->HasKey("noteBackground"))
		{
			Windows::UI::Xaml::Media::Brush^ brush;
			auto note = dynamic_cast<INote^>(psym->_sym);
			if (note)
			{
				switch (note->_voice)
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
				default:
					brush = reinterpret_cast<Windows::UI::Xaml::Media::Brush^>(_dict->Lookup("noteBackground"));
					ctrl->Background = brush;
					break;
				}
			}
		}
		break;
	case SketchMusic::SymbolType::NPART:
	case SketchMusic::SymbolType::TEMPO:
	default:
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
	
	switch (sym->GetSymType())
	{
	case SymbolType::NOTE:
	{
		INote^ inote = dynamic_cast<INote^>(sym);
		if (inote)
		{
			// TODO : сделать расчёт похитрее, чтобы выглядело как на нотном стане
			// В том числе разрещить вылезать за пределы, увеличивая тем самым высоту строки
			// (чтобы строки не налезали друг на друга)
			offsetY = inote->_val >= 0 ?
				-abs(inote->_val) % 12 :
				-12 * ((inote->_val % 12) != 0) + abs(inote->_val) % 12;
			offsetY *= RowHeight / 12;
			offsetY += RowHeight - 5;
		}
		break;
	}
	case SymbolType::GNOTE:
	{
		SGNote^ gnote = dynamic_cast<SGNote^>(sym);
		if (gnote)
		{
			offsetY = - abs(gnote->_valY) * RowHeight / 5;
			offsetY += RowHeight - 5;
			break;
		}
	}
	case SymbolType::END:
	{
		offsetY = RowHeight - 14;
		break;
	}
	default:
	{
		offsetY = -14;
		break;
	}
	}

	return offsetY;
}

void SketchMusic::View::TextRow::SetNoteOnCanvas(ContentControl ^ note)
{
	PositionedSymbol^ psym = dynamic_cast<PositionedSymbol^>(note->DataContext);
	if (psym == nullptr) return;

	auto point = GetCoordinatsOfPosition(psym->_pos);
	_canvas->SetLeft(note, point.X - PlaceholderWidth / 2);
	double offsetY = GetOffsetY(psym->_sym);
	_canvas->SetTop(note, point.Y + offsetY);
}

void SketchMusic::View::TextRow::SetSymbolView(PositionedSymbol ^ oldSymbol, PositionedSymbol^ newSymbol)
{
	auto ctrl = GetSymbolView(oldSymbol);
	ctrl->DataContext = newSymbol;
	SetNoteOnCanvas(ctrl);
}

void SketchMusic::View::TextRow::OpenPlaceholderContextDialog(ContentControl^ ctrl, Windows::Foundation::Point point)
{
	if (CurrentHolded) return;
	if (ctrl == nullptr) return;

	CurrentHolded = ctrl;
	ctrl->Background = SelectedPlaceholderBackground;
	PlaceholderContextMenu->ShowAt(ctrl, point);
}

void SketchMusic::View::TextRow::OnRightTapped(Platform::Object ^sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs ^e)
{
	// не пропускаем тач
	if (e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		return;

	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if (ctrl)
		OpenPlaceholderContextDialog(ctrl, e->GetPosition(ctrl));
}


void SketchMusic::View::TextRow::OnHolding(Platform::Object ^sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs ^e)
{
	// пропускаем только нажатие тачем
	if ((e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse) ||
		(e->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Pen))
		return;

	ContentControl^ ctrl = dynamic_cast<ContentControl^>(sender);
	if (ctrl)
		OpenPlaceholderContextDialog(ctrl, e->GetPosition(ctrl));

}
