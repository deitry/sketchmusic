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
	void HandleControlSymbols();	// если мы будем сразу все управл€ющие символы добавл€ть в controlText, то эта функци€ не понадобитс€

public:
	property float BPM;	// базовый темп
	property SScale^ scale; // базова€ гамма

	property IObservableVector<Text^>^ texts
	{
		IObservableVector<Text^>^ get() { return m_texts; }
		void set(IObservableVector<Text^>^ _texts) { m_texts = _texts; HandleControlSymbols(); }
	}

	property Text^ ControlText;

	IObservableVector<PartDefinition^>^ getParts();
	// ¬ дальнейшем сделаем controlText "хранителем" управл€ющих символов и проча€, что вли€ет непосредственно на композицию

	static CompositionData^ deserialize(Windows::Data::Json::JsonArray^ json);
	[Windows::Foundation::Metadata::DefaultOverloadAttribute]
	static CompositionData^ deserialize(Platform::String^ str);
	Windows::Data::Json::IJsonValue^ serialize();

	void ApplyParts(IObservableVector<PartDefinition^>^ parts);
	bool HasContent();

	CompositionData();
};