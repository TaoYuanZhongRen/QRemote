#include "WidgetServerSet.h"



extern ConfigFile g_ConfigFile;

WidgetServerSet::WidgetServerSet(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initFileConfig();
}

WidgetServerSet::~WidgetServerSet()
{}

void WidgetServerSet::initFileConfig()
{
	connect(ui.Button_ServerSet_Apply, &QPushButton::clicked, this, &WidgetServerSet::onButtonApply);

	ListenPort = g_ConfigFile.GetInt("Settings", "ListenPort");
	MaxConnection = g_ConfigFile.GetInt("Settings", "MaxConnection");

	ui.LineEdit_ListenPort->setText(QString::number(ListenPort));
	ui.LineEdit_MaxConnection->setText(QString::number(MaxConnection));

}

void WidgetServerSet::initChildPointer()
{
	
	
}
void WidgetServerSet::onButtonApply()
{
	g_ConfigFile.SetInt("Settings", "ListenPort", ui.LineEdit_ListenPort->text().toInt());
	g_ConfigFile.SetInt("Settings", "MaxConnection", ui.LineEdit_MaxConnection->text().toInt());
	close();
}