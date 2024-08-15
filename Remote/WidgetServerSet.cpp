#include "WidgetServerSet.h"


ConfigFile WidgetServerSet::m_ConfigFile;

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

	ListenPort = m_ConfigFile.GetInt("Settings", "ListenPort");
	MaxConnection = m_ConfigFile.GetInt("Settings", "MaxConnection");

	ui.LineEdit_ListenPort->setText(QString::number(ListenPort));
	ui.LineEdit_MaxConnection->setText(QString::number(MaxConnection));

}

void WidgetServerSet::initChildPointer()
{
	
	
}
void WidgetServerSet::onButtonApply()
{
	m_ConfigFile.SetInt("Settings", "ListenPort", ui.LineEdit_ListenPort->text().toInt());
	m_ConfigFile.SetInt("Settings", "MaxConnection", ui.LineEdit_MaxConnection->text().toInt());
	close();
}