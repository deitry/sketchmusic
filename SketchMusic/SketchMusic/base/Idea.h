#pragma once

#include "../SketchMusic.h"

using namespace SketchMusic;

public ref class SketchMusic::Idea sealed
{
	property Platform::String^ Name;		// ��� ��� ����
	property Platform::String^ Description;	// �������� ����
	property IdeaCategoryEnum Category;		// � ����� ��������� ��������� ����
	SketchMusic::Idea^	Parent;				// ������ �� ������������ ����.
		// �����, ��� � �� ���������, �� ��� �������, ��� ����� ������ ������� ����� ����� � �������� �����.
		// ����� ����� ����� ���������� ������ "��������" ����
		// TODO : "������������� ������������" ? - ���� ����� ���� �������� ����� ��� ���������� �����.
		// ������, ������ (2016.12.17) ��������������, ��� ����� ������� �������������� ������� "������� ����",
		// ������� ����� ��������� ������ ����� �� ����� �������� �����. �� ������ ������ � �������� ���� ����� ���������,
		// �� ��� ����� ��������
	SketchMusic::Text^ Content;				// ����������, � ��� ���� �����������
	Windows::Foundation::Collections::IVector<String^>^ tags;		// ������ �����, �� ������� ����� ����� ������ ����
	Windows::Foundation::Collections::IVector<String^>^ projects;	// � ����� �������� ���� �������������
};