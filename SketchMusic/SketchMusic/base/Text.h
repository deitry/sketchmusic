#pragma once

#include "../SketchMusic.h"
#include "../player/Instrument.h"
#include <map>
#include "Cursor.h"
#include <string>
#include "collection.h"

using namespace SketchMusic;
using namespace Windows::Foundation::Collections;

/**
 * ����� ���������� � ���� ���������� �������-���� � �������� ������ � ����.
 * �� ������, ��� ��� ���� ����� ��������� �����, �������� ����� �������� ������ map,
 * ��� ����� ������ ����� ������, ���� �� ����� �������� ����� �����������,
 * �������� ����� ����� ����� ����. ���� ����� ��������� ������ � �������� ���.
 * ����� ���� ����� ������� �� ����� ������ ����� ����, � �� ���.
 * �� �� �������� � ��������, ��� ��� �� ������ ������ � � �� ������ ������, ���
 * ����� ������ ������������ ��� �������� �������������.
 *
 */
[Windows::Foundation::Metadata::WebHostHiddenAttribute]
[Windows::UI::Xaml::Data::BindableAttribute]
public ref class SketchMusic::Text sealed
{
private:
	std::multimap<Cursor^, ISymbol^> _t;
		// ����� ����� ���� ��������� ��������� �������� � ����� ����������.
		// ��� ���� ����� ����� ������� ������ ��� ���, ����� ���� ���������� � �������,
		// � ��������� ������� ��������� ��� ������ ��� �������� ���� �� ������.
		// todo : ���� �� ������ �� ��������� �� ������ ��� ����� �������� ��� ���, �������? �� ����.
	
		// todo : ����� �������� ������ ��������� �������. ��� ���������� ����� ����� �������
		// ����������� �� ����� ������ � ������� ������ ����, ���-�� ��� ����

		// ��� ������� �������� ������� �� ���������� ���������, � ������������� - ������������ �������� ��������?
		// ���������� - ����� ��� �������������� ��������, ����������� ��� ���������� ��������� � ������ �� ���������...
public:
	Text();
	Text(Windows::Data::Json::JsonObject^ json) {}
	Text(Instrument^ instrument);
	//Text(String^ instrumentName);
		// TODO : ��� �� ����� ������������ ���������� ��� ����������� ������?
		// ���� ������������ ���� ����� ��� ����������� ����������� ������������ ����,
		// �� ���������� ����� ��� �� ���� ���� �����, � ����� �� ������ ������� ���� ������
		// ! ������ ���������� ��������� � ����� ������, ���� �� ������� ���������� ��, ��� � ������ ��������.
		// �������������� ��� �������� ����������� ��� ����������� ������� ������ ������������ ����� "���������" ����������

	static SketchMusic::Text^ deserialize(Platform::String^ str);

	property SketchMusic::Instrument^ instrument;	// ����������, ������� ����������� � ������ ��������
	property SketchMusic::Cursor^ curPosition;

	void addSymbol(PositionedSymbol^ sym) { _t.insert(std::make_pair(sym->_pos, sym->_sym)); }
	void addSymbol(Cursor^ cur, ISymbol^ sym) { _t.insert(std::make_pair(cur, sym)); }
	ISymbol^ getSymbol(Cursor^ cur) { return _t.find(cur)->second; }
	void deleteSymbol(Cursor^ cur, ISymbol^ sym);
	void deleteSymbols(Cursor^ begin, Cursor^ end);
	
	void moveSymbol(PositionedSymbol^ psym, SketchMusic::Cursor^ newpos);
	
	// ������ � ������ - �������, ��������
	void addBeat(Cursor^ position, Cursor^ offset);
	void deleteBeat(Cursor^ position, Cursor^ offset);

	IVector<PositionedSymbol^>^ getText();
	//Platform::Collections::Vector<PositionedSymbol^>^ getText();
	
	Windows::Data::Json::IJsonValue^ serialize();
	
	// TODO : ������� ������� ������ ������ � ������, �����������, "���������" � ��� �����
	// �����������, ����� ����� ����� ���� ���������� ���� � ������
};

