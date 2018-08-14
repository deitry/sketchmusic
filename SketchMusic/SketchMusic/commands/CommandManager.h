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
public ref class SketchMusic::Commands::CommandManager sealed //: Windows::UI::Xaml::Data::INotifyPropertyChanged
{
private:
	// ������� ������
	Windows::Foundation::Collections::IVector<SketchMusic::Commands::CommandState^>^ _commands;
	//void OnPropertyChanged(Platform::String^ propertyName);
	int m_index;
	bool m_canUndo;
	bool m_canRedo;

public:

	//virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

	// ������ �������, ������ ��� ��� �� ������� ��������� �������� �������� ����� ProertyChanged
	event EventHandler<bool>^ CanUndoChanged;
	event EventHandler<bool>^ CanRedoChanged;

	CommandManager()
	{
		MaxCommandCount = DEFAULT_MAX_COM;
		_commands = ref new Platform::Collections::Vector<CommandState^>;
		_CurrentIndex = -1;
	}

	CommandManager(int max)
	{
		MaxCommandCount = max;
		_commands = ref new Platform::Collections::Vector<CommandState^>;
		_CurrentIndex = -1;
	}

	property int MaxCommandCount; // ������������ ���������� ������
	property int _CurrentIndex	// ��������� ������ ��������� ����������� ������� (-1, ���� ������ ���)
	{
		int get() { return m_index; }
		void set(int ind)
		{
			m_index = ind;
			if (m_index >= 0)
				CanUndo = true;
			else CanUndo = false;
			//int max = ; // �� ���������� �������, ���� ������ �������� ��������� � ������������ ��������,
			if (m_index < ((int)_commands->Size - 1))
				CanRedo = true;
			else CanRedo = false;
		}
	}
	property bool CanUndo { bool get() { return m_canUndo; }
	void set(bool can)
	{
		if (m_canUndo == can) return;
		m_canUndo = can;
		//OnPropertyChanged(L"CanUndo");
		CanUndoChanged(this, m_canUndo);
	}
	}
	property bool CanRedo
	{
		bool get() { return m_canRedo; }
		void set(bool can)
		{
			if (m_canRedo == can) return;
			m_canRedo = can;
			//OnPropertyChanged(L"CanRedo");
			CanRedoChanged(this, m_canRedo);
		}
	}

	// ������ ��������� �����������
	bool Undo()
	{
		if (CanUndo)
		{
			// "�����������" ��������� ���������� �������, ��������� �������
			return _commands->GetAt(_CurrentIndex--)->Unexecute();
		};
		return false;
	}

	bool Redo()
	{
		if (CanRedo)
		{
			return ExecuteCurrent();
		}
		return false;
	}

	void Clear()
	{
		_commands->Clear();
		_CurrentIndex = -1;
	}

	// ��������� ������� ������� (��������� ��� ��, �� ������� �� �������� � ���������� undo-redo)
	bool ExecuteCurrent()
	{
		// ��������� ������� �� ��������� �������, ������ �
		return _commands->GetAt(++_CurrentIndex)->Execute();
	}

	bool AddAndExecute(SketchMusic::Commands::Command^ command, Platform::Object^ args)
	{
		auto commandState = ref new CommandState(command, args);
		// ���� �� ���������� ������ ������ - ������� ��� �����������, ����� �� ��������� ������������ �����
		if (_CurrentIndex != _commands->Size - 1)
		{
			for (int cur = _commands->Size - 1; cur > _CurrentIndex; cur--)
			{
				_commands->RemoveAtEnd();
			}
		}

		// ���� ���������� ������ �������� ��������� ������� ��� �� �����-�� �������� ��������� ���
		// - ������� ������� �� ������
		while (_CurrentIndex >= MaxCommandCount - 1)
		{
			_commands->RemoveAt(0);
			_CurrentIndex--;
		}

		_commands->Append(commandState);
		return ExecuteCurrent();
	}
};