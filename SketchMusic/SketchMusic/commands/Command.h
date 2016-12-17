#pragma once

#include "pch.h"
#include "../SketchMusic.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Input;
using namespace SketchMusic;

/*public ref class SketchMusic::Commands::Command sealed : public ICommand
{
private:
	Handler^ _handler;
	Handler^ _unexecute;
	CanExecuteMethod^ _canExecute;

public:

	Command(Handler^ handler, CanExecuteMethod^ canExecute, Handler^ unexecute)
	{
		_handler = handler;
		_canExecute = canExecute;
		_unexecute = unexecute;
	}

	virtual bool CanExecute(Object^ parameter)
	{
		if (_canExecute)
			return _canExecute(parameter);
		
		return true;
	}

	virtual void Execute(Object^ parameter)
	{
		if (_handler)
			_handler(parameter);
	}

	void UnExecute(Object^ parameter)
	{
		if (_unexecute)
			_unexecute(parameter);
	}

	virtual event EventHandler<Platform::Object^>^ CanExecuteChanged;
};*/