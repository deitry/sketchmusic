/**
Интерфейс приложения Редактор мотивов.
*/

// Можно объединить в класс состояния приложения.
class IStrokeEditor
{
	// Глобальные переменные и флаги приложения.
	bool modified; 	// изменяли ли файл с момента последнего сохранения
	// list<SketchMusic::App::Command> _history; // история команд - для ундо-редо
	
	// вызываемые методы
	void onSaveButtonClick();
	virtual void onLoadButtonClick() = 0;
	
	virtual void onUndoButtonClick() = 0;
	virtual void onRedoButtonClick() = 0;
};
