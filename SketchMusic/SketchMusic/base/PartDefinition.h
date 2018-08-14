#pragma once

#include "../SketchMusic.h"
#include "SNewPart.h"

/**
	������ ������������� PositionedSymbol ��� ������������ �� ������ ������ � ��������� ����������,
	����� ��������� ����������� ������, ��� ����� ���������������
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

	property int Number		// ���������� ����� ����� � ���� ����������. ����� ����������� �������������, ����� ������ ��� ��������
	{
		int get() { return m_number; }
		void set(int num) { m_number = num; OnPropertyChanged("Number"); }
	}
	property int Length	// ����������������� � "�����". ����� ����������� �������������, ����� ������ ��� ��������
	{
		int get() { return m_length; }
		void set(int num) { m_length = num; OnPropertyChanged("Length"); }
	}
	property double Time
	{
		double get() { return m_time; }
		void set(double num) { m_time = num; OnPropertyChanged("Time"); }
	}

	// ������������ ����� INotifyPropertyChanged
	virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;
	// ����������������� � ��������, ����� �� ��� ��������. ����� ��������� ������������, ��������� �� ������ � �������� STempo
};