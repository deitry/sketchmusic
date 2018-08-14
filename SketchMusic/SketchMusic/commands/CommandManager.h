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
public ref class SketchMusic::Commands::CommandManager sealed //: Windows::UI::Xaml::Data::INotifyPropertyChanged
{
private:
	// История команд
	Windows::Foundation::Collections::IVector<SketchMusic::Commands::CommandState^>^ _commands;
	//void OnPropertyChanged(Platform::String^ propertyName);
	int m_index;
	bool m_canUndo;
	bool m_canRedo;

public:

	//virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

	// делаем вручную, потому что мне не удалось заставить работать привязки через ProertyChanged
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

	property int MaxCommandCount; // максимальное количество команд
	property int _CurrentIndex	// указывает индекс последней выполненной команды (-1, если ничего нет)
	{
		int get() { return m_index; }
		void set(int ind)
		{
			m_index = ind;
			if (m_index >= 0)
				CanUndo = true;
			else CanUndo = false;
			//int max = ; // по непонятной причине, если просто вставить выражение в возвращаемое значение,
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

	// список доступных манипуляций
	bool Undo()
	{
		if (CanUndo)
		{
			// "разделываем" последнюю выполненую команду, уменьшаем счётчик
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

	// Выполняет текущую команду (последнюю или ту, на которую мы отмотали в результате undo-redo)
	bool ExecuteCurrent()
	{
		// переводим счётчик на следующую команду, делаем её
		return _commands->GetAt(++_CurrentIndex)->Execute();
	}

	bool AddAndExecute(SketchMusic::Commands::Command^ command, Platform::Object^ args)
	{
		auto commandState = ref new CommandState(command, args);
		// если мы посередине списка команд - убираем все последующие, чтобы не создавать параллельных веток
		if (_CurrentIndex != _commands->Size - 1)
		{
			for (int cur = _commands->Size - 1; cur > _CurrentIndex; cur--)
			{
				_commands->RemoveAtEnd();
			}
		}

		// если количество команд достигло заданного предела или по каким-то причинам превышает его
		// - удаляем команду из начала
		while (_CurrentIndex >= MaxCommandCount - 1)
		{
			_commands->RemoveAt(0);
			_CurrentIndex--;
		}

		_commands->Append(commandState);
		return ExecuteCurrent();
	}
};