//
// TextRow.xaml.h
// Объявление класса TextRow.
//

#pragma once

#include "../SketchMusic.h"
#include "../base/Cursor.h"
#include "Generated Files\SketchMusic\view\TextRow.g.h"

using namespace SketchMusic;
using namespace Windows::UI::Xaml::Controls;

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::View::TextRow sealed //: public Windows::UI::Xaml::Controls::Control
{
private:
	double BeatWidth;
	double PlaceholderWidth;
	double RowHeight;
	double SymbolHeight;


	CompositionData^ data;	// список нужен для отображения нескольких дорожек 

	int currentEditing;	// индекс текста, с которым идёт основная работа
	int currentFormat;	// индекс текста, чьё форматирование берётся за основу

	std::vector<Windows::UI::Xaml::Controls::ContentControl^> notes;
	Windows::UI::Xaml::ResourceDictionary^ _dict;
	ContentControl^ _dragged;
	ContentControl^ CurrentHolded;	// для выделения удерживаемого плейсхолдера
	Windows::Foundation::Point _lastPoint;
	ContentControl^ _currentSnapPoint;
	ContentControl^ _snapPoint;
	bool _isMoving;
	bool _isPressed;

	// 1..32, 1 - 1 ктрл - одна доля, 32 - на одну долю 32 ктрла
	int initialised;	// до какой степени масштаба инициализировано

						//Cusror^ begin; // для выравнивания - перед положением курсора, отмеченным началом,
						// могут быть ещё символы

	Cursor^ _maxPos;
	Cursor^ _startPos;	// что принимаем за ноль
	int m_quantize;

	std::vector<PositionedSymbol^> breakLine;
	// в отдельном массиве содержим символы, которые отмечают разрыв строки.
	// В том числе здесь могут содержаться символы, определяющие вынужденный разрыв в результате того, закончилась ширина

internal:
	// вспомогательные функции для удобной работы с символами
	ContentControl^ GetControlAtPos(Cursor^ pos, int offset = 0);
	int GetControlIndexAtPos(Cursor^ pos, int offset);

	float CalculateTick(float offsetX, ContentControl^ ctrl);
	Cursor^ GetPositionOfControl(ContentControl^ ctrl, Point offset);
	Point GetCoordinatsOfPosition(Cursor^ pos);
	Point GetCoordinatsOfControl(ContentControl^ ctrl, Point offset);
	void InvalidateText();
	void RedrawText();
	void AllocateSnapPoints(Text^ text, int newScale, int selectedPart);
	void InsertLineBreak(Cursor^ pos);
	void DeleteLineBreak(Cursor^ pos);
	void SetBackgroundColor(ContentControl^ ctrl);
	void OpenPlaceholderContextDialog(ContentControl^ ctrl, Windows::Foundation::Point point);

	double GetOffsetY(ISymbol^ sym);
	void SetNoteOnCanvas(ContentControl^ note);

public:
	// команды для манипуляциии с текстом
	property SketchMusic::Commands::CommandManager^ _CommandManager; // Передаём извне, поскольку общий для приложения
	property SketchMusic::Commands::Command^ MoveSymCommand;	// перемещение символа
	property SketchMusic::Commands::Command^ AddSymCommand;		// добавление нового символа
	property SketchMusic::Commands::Command^ DeleteSymCommand;	// удаление символа
	property SketchMusic::Commands::Command^ AddMultipleSymCommand;	// удаление символа
	property SketchMusic::Commands::Command^ DeleteMultipleSymCommand;	// удаление символа
	property SketchMusic::Commands::Command^ AddBeatCommand;	// удаление символа
	property SketchMusic::Commands::Command^ DeleteBeatCommand;	// удаление символа

	property SketchMusic::Commands::Handler^ AddBeatHandler;
	property SketchMusic::Commands::Handler^ DeleteBeatHandler;

	property Cursor^ currentPosition;
	property Text^ current;
	property Text^ format;
	property int scale;			// текущий уровень масштаба 
	property int quantize		// текущий уровень квантизации - делитель Tick
	{
		int get() { return m_quantize; }
		void set(int val) { m_quantize = val ? val : 1; }
	}
	property bool EraserTool;	// если true - по нажатию на кнопку удаляем

	TextRow();
	void InitializePage();

	// универсальная функция
	void SetText(CompositionData^ textCollection, SketchMusic::Text^ current);
	void SetText(CompositionData^ textCollection, SketchMusic::Text^ current, int selectedPart);
	SketchMusic::Text^ GetText();
	void SetCursor(SketchMusic::Cursor^ pos);

	// работа с отдельными символами
	void AddSymbolView(Text^ source, PositionedSymbol^ symbol);
	void DeleteSymbolView(PositionedSymbol^ symbol);
	ContentControl^ GetSymbolView(PositionedSymbol^ symbol);
	void SetSymbolView(PositionedSymbol^ oldSymbol, PositionedSymbol^ newSymbol);

	// отображение
	void SetScale(int scaleFactor);
	void MoveCursorLeft();
	void MoveCursorRight();

	// обработка событий
	void OnPointerWheelChanged(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void HighlightSnapPoint(Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerMoved(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerPressed(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);

	void AddBeatItem_Click(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
	void DeleteBeatItem_Click(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
	void PlaceholderContextMenuOnClosed(Object^ sender, Platform::Object^ args);

	event Windows::Foundation::EventHandler<PositionedSymbol^>^ SymbolPressed;
	// как будет осуществляться вставка долей?
	// легче всего, создав класс реализующий ISymbol и передавать как обычно.
	// Необычным будет только действие на текст
	void OnRightTapped(Platform::Object ^sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs ^e);
	void OnHolding(Platform::Object ^sender, Windows::UI::Xaml::Input::HoldingRoutedEventArgs ^e);
};

