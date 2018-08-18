#pragma once

#include "../SketchMusic.h"
#include "SScale.h"

using namespace SketchMusic;
using namespace Platform;
using namespace Windows::Foundation::Collections;


[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class SketchMusic::CompositionData sealed
{
private:
	IObservableVector<Text^>^ m_texts;
	void HandleControlSymbols();	// ���� �� ����� ����� ��� ����������� ������� ��������� � controlText, �� ��� ������� �� �����������

public:
	property float BPM;	// ������� ����
	property SScale^ scale; // ������� �����

	property IObservableVector<Text^>^ texts
	{
		IObservableVector<Text^>^ get() { return m_texts; }
		void set(IObservableVector<Text^>^ _texts) { m_texts = _texts; HandleControlSymbols(); }
	}

	property Text^ ControlText;

	IObservableVector<PartDefinition^>^ getParts();
	// � ���������� ������� controlText "����������" ����������� �������� � ������, ��� ������ ��������������� �� ����������

	static CompositionData^ deserialize(Windows::Data::Json::JsonArray^ json);
	[Windows::Foundation::Metadata::DefaultOverloadAttribute]
	static CompositionData^ deserialize(Platform::String^ str);
	Windows::Data::Json::IJsonValue^ serialize();

	void ApplyParts(IObservableVector<PartDefinition^>^ parts);
	bool HasContent();

	CompositionData();
};