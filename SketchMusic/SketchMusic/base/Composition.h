#pragma once

#include "../SketchMusic.h"
#include "CompositionData.h"
#include "Idea.h"

using namespace SketchMusic;
using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace SketchMusic
{
	/**
	"�����������" ������ ������ Composition. ����� ��� ����������� �������
	*/
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class CompositionHeader sealed
	{
	public:
		property String^ Name;
		property String^ Description;
		property String^ FileName;
	};

	//public ref class CompositionLibrary sealed
	//{
	//	IVector<PositionedIdea^>^ Ideas;
	//};

	/**
	������� ���������� ��� ����������
	*/
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class Composition sealed
	{
	public:
		property CompositionHeader^ Header;
		property CompositionData^ Data;
		//property CompositionLibrary^ Lib;
		property IVector<PositionedIdea^>^ Ideas;

		Windows::Data::Json::IJsonValue^ Serialize();
		static Composition^ Deserialize(Windows::Data::Json::JsonObject^ json);

		Composition() { Header = ref new CompositionHeader; Data = ref new CompositionData; Ideas = ref new Platform::Collections::Vector<SketchMusic::PositionedIdea^>; }
	};

	/**
	������������� �����, ������������ ����� ����������.
	��� ����� ����� � ��� ������ ��������� ����������� ����������, ��������� �� ���� �� ����� ��������� �� ��� ������������.
	*/
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class Album sealed
	{
	public:
		property String^ name;
		property Windows::Foundation::Collections::IVector<CompositionHeader^>^ _compositions;
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class Project sealed
	{
		//private :
	public:
		property String^ name;
		property Windows::Foundation::Collections::IVector<Album^>^ _albums;
	};
}