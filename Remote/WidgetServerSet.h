#pragma once

#include <QWidget>
#include "ConfigFile.h"
#include "ui_WidgetServerSet.h"

class WidgetServerSet : public QWidget
{
	Q_OBJECT

public:
	WidgetServerSet(QWidget *parent = nullptr);
	~WidgetServerSet();

	void initFileConfig();
	void initChildPointer();

	void onButtonApply();



public:

	static ConfigFile m_ConfigFile;
private:
	Ui::WidgetServerSetClass ui;
	QPushButton* Button_ServerSet_Apply;

	int ListenPort;
	int MaxConnection;
};
