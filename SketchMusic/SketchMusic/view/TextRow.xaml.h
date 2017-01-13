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
	Windows::Foundation::Point _lastPoint;
	ContentControl^ _currentSnapPoint;
	ContentControl^ _snapPoint;
	bool _isMoving;

	// 1..32, 1 - 1 ктрл - одна доля, 32 - на одну долю 32 ктрла
	int initialised;	// до какой степени масштаба инициализировано

						//Cusror^ begin; // для выравнивания - перед положением курсора, отмеченным началом,
						// могут быть ещё символы

	Cursor^ _maxPos;

	std::vector<PositionedSymbol^> breakLine;
	// в отдельном массиве содержим символы, которые отмечают разрыв строки.
	// В том числе здесь могут содержаться символы, определяющие вынужденный разрыв в результате того, закончилась ширина

internal:
	// вспомогательные функции для удобной работы с символами
	//void InsertSpaceSymbol(PositionedSymbol^ symbol);	// нужно будет не просто делать один из плейсхолдеров шире,
	// но ещё и перемещать "пробел", если в результате масштаба данный плейсхолдер окажется невидимым
	//void CreateNoteObject(ISymbol^ note);
	void InsertNoteObject(PositionedSymbol^ note);
	ContentControl^ GetControlAtPos(Cursor^ pos, int offset = 0);
	int GetControlIndexAtPos(Cursor^ pos, int offset);

	float CalculateTick(float offsetX, ContentControl^ ctrl);
	Cursor^ GetPositionOfControl(ContentControl^ ctrl, Point offset);
	Point GetCoordinatsOfPosition(Cursor^ pos);
	Point GetCoordinatsOfControl(ContentControl^ ctrl, Point offset);
	void InvalidateText();
	void RedrawText();
	void AllocateSnapPoints(Text^ text, int newScale);
	void SetSnapPointsVisibility(int newScale);
	void InsertLineBreak(Cursor^ pos);
	void DeleteLineBreak(Cursor^ pos);
	void SetBackgroundColor(ContentControl^ ctrl);

	double GetOffsetY(ISymbol^ sym);
	void SetNoteOnCanvas(ContentControl^ note);

public:
	property Cursor^ currentPosition;
	property Text^ current;
	property Text^ format;
	property int scale;			// текущий уровень масштаба 
	property int quantize;		// текущий уровень квантизации - делитель Tick

	TextRow();
	void OnApplyTemplate() override;
	void InitializePage();

	void SetText(SketchMusic::Text^ text);
	//void SetFormat(SketchMusic::Text^ format);	// оставляем возможность ноты брать из одного текста, 
	// а формат из другого - чтобы легче было работать с несколькими дорожками	SketchMusic::Text^ GetText();

	// универсальная функция
	void SetText(CompositionData^ textCollection, SketchMusic::Text^ format);
	SketchMusic::Text^ GetText();
	void SetCursor(SketchMusic::Cursor^ pos);

	// работа с отдельными символами
	void AddSymbol(Text^ source, PositionedSymbol^ symbol);
	//void DeleteSymbol(PositionedSymbol^ symbol);
	void Backspace();
	//void DeleteRange(Cursor^ from, Cursor^ to);
	//void MoveSymbol(PositionedSymbol^ newSymbol, PositionedSymbol^ oldSymbol);

	// как будет осуществляться вставка долей?
	// легче всего, создав класс реализующий ISymbol и передавать как обычно.
	// Необычным будет только действие на текст

	// отображение
	void SetScale(int scaleFactor);
	void MoveCursorLeft();
	void MoveCursorRight();

	// обработка событий
	void OnPointerWheelChanged(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerMoved(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerPressed(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnPointerReleased(Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
	void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);

	// события
	event EventHandler<SketchMusic::Commands::SymbolHandlerArgs^>^ SymbolAdded;
	event EventHandler<SketchMusic::Commands::SymbolHandlerArgs^>^ SymbolMoved;
	event EventHandler<SketchMusic::Commands::SymbolHandlerArgs^>^ SymbolDeleted;

	// как будет осуществляться вставка долей?
	// легче всего, создав класс реализующий ISymbol и передавать как обычно.
	// Необычным будет только действие на текст
};

