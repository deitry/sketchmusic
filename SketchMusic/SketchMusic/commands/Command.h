#pragma once

#include "pch.h"
#include "../SketchMusic.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Input;
using namespace SketchMusic;

public ref class SketchMusic::Commands::Command sealed
{
private:
	Handler ^ _Handler;
	Handler^ _Unexecute;
	CanExecuteMethod^ _CanExecute;

public:
	Command(Handler^ handler, Handler^ unexecute)
	{
		_Handler = handler;
		_Unexecute = unexecute;
	}

	Command(Handler^ handler, Handler^ unexecute, CanExecuteMethod^ canExecute)
	{
		_Handler = handler;
		_CanExecute = canExecute;
		_Unexecute = unexecute;
	}

internal:

	virtual bool CanExecute(Object^ parameter)
	{
		if (_CanExecute)
			return _CanExecute(parameter);
		return true;
	}

	virtual bool Execute(Object^ parameter)
	{
		if (_Handler) // && CanExecute
			return _Handler(parameter);
		return false;
	}

	virtual bool Unexecute(Object^ parameter)
	{
		if (_Unexecute)
			return _Unexecute(parameter);
		return false;
	}

	virtual event EventHandler<Platform::Object^>^ CanExecuteChanged;
};

// команда вместе с её параметром
public ref class SketchMusic::Commands::CommandState sealed
{
public:
	CommandState(Command^ command, Object^ args)
	{
		_command = command;
		_args = args;
	}

	// обёртка, чтобы не морочиться с аргументами
	// TODO : возвращать bool - выполнена команда или нет
	bool Execute() { return _command->Execute(_args); }
	bool Unexecute() { return _command->Unexecute(_args); }

	property Command^ _command;
	property Object^ _args;
};