#include "pch.h"
#include "Sample.h"

SketchMusic::Sample::Sample(Platform::String^ FileName)
{
	// - ���������� �� ������ ������
	// - ���� ���� � ���������� ���������
	// - - ���� ��� - ���������

	// - ���� �� ����� - ���������� ����� �� ���������
	this->_name = "Square";
	this->_frequency = 441;
	this->_length = static_cast<int>((int) DISCRETIZATION_FREQ / this->_frequency * 2);
	_sample = ref new Platform::Array<uint8>(this->_length);
	
	for (int i = 0; i < this->_length; )
	{
		short value = (i < this->_length / 2) ? 32767 : -32768;
		_sample->set(i++, value & 0xFF);
		_sample->set(i++, (value >> 8) & 0xFF);
	}
}

SketchMusic::Sample::~Sample()
{

}