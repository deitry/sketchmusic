// Class1.cpp
#include "pch.h"
#include "SketchMusic.h"

using namespace SketchMusic;
using namespace Platform;

Platform::String^ SketchMusic::SNote::valToString(int val)
{
	return "0";
}

Platform::String^ SketchMusic::SRNote::valToString(int val)
{
	return "0";
}

Platform::String^ SketchMusic::SGNote::valToString(int val)
{
	return "0";
}

ISymbol ^ SketchMusic::ISymbolFactory::Deserialize(String ^ string)
{
	// прочитать первый литерал до "," если таковая имеется или до конца
	// - на основании прочитанного сделать вывод о типе будущей ноты
	int i = 0;
	//for (auto ch = string->Data(); ch != '\0'; ch++, i++)
	auto ch = string->Data();
	while (ch)
	{
		ch++;
		if ((*ch == ',') || (*ch=='\0'))
		{
			Platform::String^ type = ref new String(ch - i, i);
		}
		
	}
	// если есть запятая, прочитать текст после неё
	// - это значение, приписанное ноте
	// - если нету, то либо оно изначально не требуется, либо это ошибка
	// - stroked вообще нужен? если что, иметь его в виду
	return nullptr;
}
