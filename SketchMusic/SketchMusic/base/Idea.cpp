#include <time.h>  

#include "pch.h"
#include "Idea.h"

using namespace SketchMusic;
using namespace Windows::Data::Json;
namespace t = SketchMusic::SerializationTokens;

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
	CompositionData^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified)
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
	String ^ _content, String ^ _tags, String ^ _projects, long long _created, long long _modified)
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

CompositionData^ SketchMusic::Idea::GetContent()
{
	if ((Content == nullptr) && (SerializedContent != nullptr))
	{
		//Content = SketchMusic::Text::deserialize(SerializedContent);
		//SerializedContent = nullptr;	// чтобы не казалось, что мы можем просто взять это значение
			// Предполагается, что если мы всё это делаем, то хотим изменить идею.
	}
	return Content;
}

Windows::Data::Json::IJsonValue ^ SketchMusic::PositionedIdea::Serialize()
{
	auto json = ref new Windows::Data::Json::JsonObject;
	json->Insert(t::IDEA_HASH, JsonValue::CreateNumberValue(Content->Hash));
	if (Pos && Pos->Beat)				json->Insert(t::PIDEA_POS, JsonValue::CreateNumberValue(Pos->Beat));
	if (Layer)					json->Insert(t::PIDEA_LAY, JsonValue::CreateNumberValue(Layer));
	if (Length && Length->Beat)			json->Insert(t::PIDEA_LEN, JsonValue::CreateNumberValue(Length->Beat));
	if (Content && (int)Content->Category) json->Insert(t::IDEA_CAT, JsonValue::CreateNumberValue((int)Content->Category));
	if (Content && Content->Name)			json->Insert(t::IDEA_NAME, JsonValue::CreateStringValue(Content->Name));
	if (Content && Content->SerializedContent) json->Insert(t::IDEA_CONTENT, JsonValue::CreateStringValue(Content->SerializedContent));
	if (Content && Content->Rating)		json->Insert(t::IDEA_RATE, JsonValue::CreateNumberValue(Content->Rating));
	if (Content && Content->CreationTime)	json->Insert(t::IDEA_CRE, JsonValue::CreateNumberValue(static_cast<double>(Content->CreationTime)));
	if (Content && Content->ModifiedTime)	json->Insert(t::IDEA_MOD, JsonValue::CreateNumberValue(static_cast<double>(Content->ModifiedTime)));
	if (Content && Content->Description)	json->Insert(t::IDEA_DESC, JsonValue::CreateStringValue(Content->Description));
	if (Content && Content->Parent)		json->Insert(t::IDEA_PARENT, JsonValue::CreateNumberValue(Content->Parent));
	if (Content && Content->Tags)			json->Insert(t::IDEA_TAGS, JsonValue::CreateStringValue(Content->Tags));

	return json;
}

PositionedIdea ^ SketchMusic::PositionedIdea::Deserialize(Windows::Data::Json::JsonObject ^ json)
{
	auto idea = ref new PositionedIdea();
	idea->Pos = ref new Cursor((int)json->GetNamedNumber(t::PIDEA_POS, 0));
	idea->Layer = static_cast<int>(json->GetNamedNumber(t::PIDEA_LAY, 0));
	idea->Length = ref new Cursor((int)json->GetNamedNumber(t::PIDEA_LEN, 0));

	idea->Content = ref new Idea;
	idea->Content->Hash = (int)json->GetNamedNumber(t::IDEA_HASH, 0);
	idea->Content->Category = (IdeaCategoryEnum) (int) json->GetNamedNumber(t::IDEA_CAT, 0);
	idea->Content->Name = json->GetNamedString(t::IDEA_NAME, nullptr);
	idea->Content->SerializedContent = json->GetNamedString(t::IDEA_CONTENT, nullptr);
	idea->Content->Rating = (int)json->GetNamedNumber(t::IDEA_RATE, 0);
	idea->Content->CreationTime = (long long) json->GetNamedNumber(t::IDEA_CRE, 0);
	idea->Content->ModifiedTime = (long long) json->GetNamedNumber(t::IDEA_MOD, 0);
	idea->Content->Description = json->GetNamedString(t::IDEA_DESC, nullptr);
	idea->Content->Parent = (int)json->GetNamedNumber(t::IDEA_PARENT, 0);
	idea->Content->Tags = json->GetNamedString(t::IDEA_TAGS, nullptr);

	return idea;
}
