#include "pch.h"
#include "Idea.h"

//SketchMusic::Idea::Idea()
//{
//	// ���������������� ���������? ������, ������ ����� ����� ������������� �� ������������ �� �����
//}

SketchMusic::Idea::Idea(String ^ _name, IdeaCategoryEnum _cat) //int _cat)
{
	this->Name = _name;
	this->Category = _cat;
}
