/**
��������� ���������� �������� �������.
*/

// ����� ���������� � ����� ��������� ����������.
class IStrokeEditor
{
	// ���������� ���������� � ����� ����������.
	bool modified; 	// �������� �� ���� � ������� ���������� ����������
	// list<SketchMusic::App::Command> _history; // ������� ������ - ��� ����-����
	
	// ���������� ������
	void onSaveButtonClick();
	virtual void onLoadButtonClick() = 0;
	
	virtual void onUndoButtonClick() = 0;
	virtual void onRedoButtonClick() = 0;
};
