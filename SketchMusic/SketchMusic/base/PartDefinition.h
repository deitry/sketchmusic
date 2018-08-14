#pragma once

#include "../SketchMusic.h"
#include "SNewPart.h"

/**
	Вместо использования PositionedSymbol для оперирования на уровне частей в редакторе композиции,
	будем создавать специальный объект, для этого предназначенный
*/
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::PartDefinition sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
{
private:
	void OnPropertyChanged(Platform::String^ propertyName);
	int m_number;
	int m_length;
	double m_time;
public:
	PartDefinition() {}
	PartDefinition(PositionedSymbol^ psym) { originalPos = psym->_pos; original = dynamic_cast<SNewPart^>(psym->_sym); }
	PartDefinition(Cursor^ pos, SNewPart^ sym) { originalPos = pos; original = sym; }

	PositionedSymbol^ GetPositionedSymbol(Cursor^ cur)
	{
		if (cur) return ref new PositionedSymbol(cur, original);
		else return ref new PositionedSymbol(originalPos, original);
	}

	property Cursor^ originalPos;
	property SNewPart^ original;

	property int Number		// порядковый номер части с этой категорией. Будет вычисляться автоматически, здесь храним для удобства
	{
		int get() { return m_number; }
		void set(int num) { m_number = num; OnPropertyChanged("Number"); }
	}
	property int Length	// продолжительность в "битах". Будет вычисляться автоматически, здесь храним для удобства
	{
		int get() { return m_length; }
		void set(int num) { m_length = num; OnPropertyChanged("Length"); }
	}
	property double Time
	{
		double get() { return m_time; }
		void set(double num) { m_time = num; OnPropertyChanged("Time"); }
	}

	// Унаследовано через INotifyPropertyChanged
	virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;
	// продолжительность в секундах, опять же для удобства. Будем вычислять автоматичеки, проходясь по тексту и учитывая STempo
};