#pragma once

#include "pch.h"
#include "../SketchMusic.h"
#include "Command.h"


/**
�����, ������������ ������� �������� �� ��������������� �������
� ������ �������� �� ��, ��� ������ ����� ���������, ����������� SketchMusic.

������, ��-��������, ���� ����� ������ ���� ��������� �� ��������� ������������ SketchMusic,
��� ��� ���������� ����� ���� ������. ������, ����� �� ������� ������� � ����� � SketchMusic
���� ��������� ���������� �� ���������, ��������� �� ���.

�� ���� ���������� ����������� � ���� �� ������������ ��� �������
*/
/*public ref class SketchMusic::Commands::CommandManager sealed
{
private:
	// ������� ������
	Windows::Foundation::Collections::IVector<SketchMusic::Commands::CommandState^>^ _commands;
	int position;	// ������� ��������� � ������� ������

public:
	CommandManager()
	{
		_commands = ref new Platform::Collections::Vector<CommandState^>;
	}

	// ������ ��������� �����������
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