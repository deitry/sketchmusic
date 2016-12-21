#pragma once

#include "../SketchMusic.h"
#include "../player/Instrument.h"
#include <map>
#include "Cursor.h"
#include <string>
#include "collection.h"
//#include "rapidjson/prettywriter.h"
//#include "rapidjson/reader.h"

using namespace SketchMusic;
using namespace Windows::Foundation::Collections;

/*public ref class SketchMusic::TextToJsonConverter sealed
{
internal:
	template <typename Writer>
	void Serialize(Writer& writer) const {
	}

	virtual Object^ Deserialize(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
	{
		return nullptr;
		//return ref new SketchMusic::PositionedSymbol(ref new SketchMusic::Cursor, ref new SketchMusic::SNote);
	}

	TextToJsonConverter() {}
};*/

/*struct SmTextParserHandler {
	bool Null() { return true; }
	bool Bool(bool b) { return true; }
	bool Int(int i) { return true; }
	bool Uint(unsigned u) { return true; }
	bool Int64(int64_t i) { return true; }
	bool Uint64(uint64_t u) { return true; }
	bool Double(double d) { return true; }
	bool RawNumber(const char* str, SizeType length, bool copy) {
		return true;
	}
	bool String(const char* str, SizeType length, bool copy) {
		return true;
	}
	bool StartObject() { return true; }
	bool Key(const char* str, SizeType length, bool copy) {
		return true;
	}
	bool EndObject(SizeType memberCount) { return true; }
	bool StartArray() { return true; }
	bool EndArray(SizeType elementCount) { return true; }
};*/

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
	static Text^ deserializeFromString(Platform::String^ serialized);	// ����������� �� ������ ��� ������� :( TODO ����� ����� ����� ��������

	Text();
	Text(Windows::Data::Json::JsonObject^ json) {}
	Text(Instrument^ instrument);
	Text(String^ instrumentName);
		// TODO : ��� �� ����� ������������ ���������� ��� ����������� ������?
		// ���� ������������ ���� ����� ��� ����������� ����������� ������������ ����,
		// �� ���������� ����� ��� �� ���� ���� �����, � ����� �� ������ ������� ���� ������
		// ! ������ ���������� ��������� � ����� ������, ���� �� ������� ���������� ��, ��� � ������ ��������.
		// �������������� ��� �������� ����������� ��� ����������� ������� ������ ������������ ����� "���������" ����������

	property SketchMusic::Instrument^ instrument;	// ����������, ������� ����������� � ������ ��������
	property SketchMusic::Cursor^ curPosition;
	///property Platform::String^ Ops;

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
	//Platform::String^ toXml();
	
	Platform::String^ serializeToString();	// �� ����, ��� ����� ������������ xml
	Windows::Data::Json::IJsonValue^ serialize();
	void deserialize(Platform::String^ str);
	// TODO : ������� ������� ������ ������ � ������, �����������, "���������" � ��� �����
	// �����������, ����� ����� ����� ���� ���������� ���� � ������
};

