#include <time.h>  

#include "pch.h"
#include "Idea.h"

// Создание "голой" идеи
SketchMusic::Idea::Idea()
{
	// получение текущего времени
	__time64_t long_time;
	_time64(&long_time);

	this->CreationTime = long_time;
	this->ModifiedTime = long_time;

	// преобразование времени в хэш
	unsigned int _hash = 0;
	for (; long_time; long_time = long_time >> 8)
	{
		_hash = (_hash * 1664525) + long_time & 0xFF + 1013904223;
	}
	this->Hash = _hash;
}

// Создание "голой" проименованной идеи
SketchMusic::Idea::Idea(String ^ _name, IdeaCategoryEnum _cat)
{
	// получение текущего времени
	__time64_t long_time;
	_time64(&long_time);

	this->CreationTime = long_time;
	this->ModifiedTime = long_time;

	// преобразование времени в хэш
	unsigned int _hash = 0;
	for (; long_time; long_time = long_time >> 8)
	{
		_hash = (_hash * 1664525) + long_time & 0xFF + 1013904223;
	}
	this->Hash = _hash;

	this->Name = _name;
	this->Category = _cat;
}

// "Загрузка" идеи из данных о ней
SketchMusic::Idea::Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
						SketchMusic::Text ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified)
{
	Hash = _hash;
	Name = _name;
	Category = _cat;
	Description = _desc;
	Parent = _parent;
	Rating = _rating;
	Content = _content;
	Tags = _tags;
	Projects = _projects;
	CreationTime = _created;
	ModifiedTime = _modified;
}

// "Загрузка" идеи из данных о ней
SketchMusic::Idea::Idea(int _hash, String ^ _name, IdeaCategoryEnum _cat, String ^ _desc, int _parent, byte _rating,
	Platform::String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified)
{
	Hash = _hash;
	Name = _name;
	Category = _cat;
	Description = _desc;
	Parent = _parent;
	Rating = _rating;
	SerializedContent = _content;
	Tags = _tags;
	Projects = _projects;
	CreationTime = _created;
	ModifiedTime = _modified;
}

SketchMusic::Text ^ SketchMusic::Idea::GetContent()
{
	if ((Content == nullptr) && (SerializedContent != nullptr))
	{
		Content = ref new Text();
		Content->deserialize(SerializedContent);
		SerializedContent = nullptr;	// чтобы не казалось, что мы можем просто взять это значение
			// Предполагается, что если мы всё это делаем, то хотим изменить идею.
	}
	return Content;
}
