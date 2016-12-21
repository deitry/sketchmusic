#include "pch.h"
#include "Cursor.h"



/*
SketchMusic::Cursor::Cursor()
{
}
*/

SketchMusic::Cursor^ SketchMusic::Cursor::inc()
{
	_tick++;
	
	if (_tick > TICK_IN_BEAT)
	{
		_tick = 0;
		_beat++;
	}
	return (ref new SketchMusic::Cursor(_beat, _tick));
}

SketchMusic::Cursor^ SketchMusic::Cursor::dec()
{
	int beat = _beat, tick = _tick--;

	if (tick < 0)
	{
		tick = TICK_IN_BEAT;
		beat--;
	}
	return (ref new SketchMusic::Cursor(beat, tick));
}

bool SketchMusic::Cursor::EQ(SketchMusic::Cursor^ that)
{
	if ((this->_beat == that->_beat) && (this->_tick == that->_tick))
	{
		return true;
	}
	return false;
}

bool SketchMusic::Cursor::GT(SketchMusic::Cursor^ that)
{
	if (this->_beat > that->_beat) return true;
	if ((this->_beat == that->_beat) && (this->_tick > that->_tick)) return true;
	return false;
}

bool SketchMusic::Cursor::LT(SketchMusic::Cursor^ that)
{
	if (this->_beat < that->_beat) return true;
	if ((this->_beat == that->_beat) && (this->_tick < that->_tick)) return true;
	return false;
}

IJsonValue ^ SketchMusic::Cursor::Serialize()
{
	return nullptr;
}
