#include "pch.h"
#include "Idea.h"

//SketchMusic::Idea::Idea()
//{
//	// инициализировать коллекции? Вообще, скорее всего таким конструктором мы пользоваться не будем
//}

SketchMusic::Idea::Idea(String ^ _name, IdeaCategoryEnum _cat) //int _cat)
{
	this->Name = _name;
	this->Category = _cat;
}
