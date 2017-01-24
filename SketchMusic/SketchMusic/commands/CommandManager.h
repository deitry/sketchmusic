#pragma once

#include "pch.h"
#include "../SketchMusic.h"
#include "Command.h"


/**
Класс, определяющий базовые операции по манипулированию текстом
и вообще содержит всё то, что должна уметь программа, реализующая SketchMusic.

Вообще, по-хорошему, этот класс должен быть независим от основного пространства SketchMusic,
ибо его реализация может быть разная. Однако, чтобы не плодить проекты и чтобы у SketchMusic
была некоторая реализация по умолчанию, оставляем всё тут.

На вход передаются определённые в этом же пространстве имён команды
*/
/*public ref class SketchMusic::Commands::CommandManager sealed
{
private:
	// История команд
	Windows::Foundation::Collections::IVector<SketchMusic::Commands::CommandState^>^ _commands;
	int position;	// текущее положение в истории команд

public:
	CommandManager()
	{
		_commands = ref new Platform::Collections::Vector<CommandState^>;
	}

	// список доступных манипуляций
	void Undo() {}
	bool CanUndo() { return true; }

	void Redo() {}
	bool CanRedo() { return true; }

	void ExecuteCurrent()
	{
		auto command = _commands->GetAt(_commands->Size - 1);
		command->_command->Execute(command->_args);
	}

	void AddCommand(SketchMusic::Commands::CommandState^ command)
	{
		_commands->Append(command);
	}
	//bool 
};*/