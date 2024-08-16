#include "Remote.h"
#include "IOCPServer.h"
#include "ConfigFile.h"

#pragma execution_character_set("utf-8")

ulong m_ConnectCount = 0;
//����ȫ��ͨ�Ŷ���
IOCPServer* g_IOCPServer = NULL;

//����ȫ���ļ����ö���
ConfigFile g_ConfigFile;



Remote::Remote(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);
	this->setWindowIcon(QIcon(".\\PicRes\\Remote"));
	this->setWindowTitle("RemoteServer");

	initChildPointer();
	initListWidget();
	initMenu();
	initStatusBar(); 
	initToolBar();

	m_ListenPort = g_ConfigFile.GetInt("Settings", "ListenPort");
	m_MaxConnectCount = g_ConfigFile.GetInt("Settings", "MaxConnection");
	ServerStart();

	//CreateSystemTrayMenu();
}

Remote::~Remote()
{
	delete g_IOCPServer;
	g_IOCPServer = NULL;
}


void Remote::initListWidget()
{
	m_TableWidget_Online->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	m_TableWidget_Online->setEditTriggers(QAbstractItemView::NoEditTriggers);//��ֹ�޸�
	m_TableWidget_Online->setSelectionMode(QAbstractItemView::ContiguousSelection);//���Զ�ѡ��
	m_TableWidget_Online->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_TableWidget_Online, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCreateTableWidgetMenu()));
	QTableWidgetItem* m_pCurrentItem = NULL;



	m_TableWidget_Message->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	m_TableWidget_Message->setEditTriggers(QAbstractItemView::NoEditTriggers);//��ֹ�޸�
	m_TableWidget_Message->setSelectionMode(QAbstractItemView::SingleSelection);//����ѡ�е���

	initTableHeader();
}
void Remote::initTableHeader()
{
	m_TableWidget_Online->setColumnCount(7);
	m_TableWidget_Message->setColumnCount(3);

	QStringList horizonHeader;
	horizonHeader << QString("IP��ַ");
	horizonHeader << QString("����");
	horizonHeader << QString("�������/��ע");
	horizonHeader << QString("����ϵͳ");
	horizonHeader << QString("CPU");
	horizonHeader << QString("����ͷ");
	horizonHeader << QString("PING");
	m_TableWidget_Online->setHorizontalHeaderLabels(horizonHeader);
	m_TableWidget_Online->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	horizonHeader.clear();

	horizonHeader << QString("��Ϣ����");
	horizonHeader << QString("ʱ��");
	horizonHeader << QString("��Ϣ����");

	m_TableWidget_Message->setHorizontalHeaderLabels(horizonHeader);
	m_TableWidget_Message->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void Remote::ServerStart()
{
	g_IOCPServer = new IOCPServer();
	if (g_IOCPServer == NULL)
	{
		return;
	}
	if (g_IOCPServer->ServerRun(m_ListenPort) == true)
	{

	}
	//���ڽ�����ʾͨ�Ŷ����Ѿ�����
	ShowDialogMessage(true, QString("�����˿ڣ�%1�ɹ�").arg(m_ListenPort));

}

void Remote::onCreateServerSetWidget()
{
	m_Widget_Server_Set = new WidgetServerSet(this);

	m_Widget_Server_Set->setWindowModality(Qt::ApplicationModal); //������������
	//configWindow->setAttribute(Qt::WA_ShowModal, true);    //�������� true:ģ̬ false:��ģ̬
	m_Widget_Server_Set->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);   //�������������Qt::Dialog������ģ̬��ʾ�᲻��Ч

	m_Widget_Server_Set->show();
}
void Remote::onTableWidgetSelectChanged()
{
	//todo ��������հ׵ط���ʱ��ȡ��tablewidget���е�ѡ�У�����Ŀǰ����������
	int columnNum = m_TableWidget_Online->columnCount();
	for (int i = 0; i < (m_TableWidget_Online->selectedItems().size())/ columnNum; i++)
	{
		for (int j = 0; j < columnNum; j++)
		{
			m_TableWidget_Online->item(i, j)->setFlags(Qt::NoItemFlags);
		}
	}
	
}
void Remote::onCreateTableWidgetMenu()
{
	QMenu menu; 
	QAction* disConnect = menu.addAction(QString("ɾ������"));
	//menu.addSeparator();
	QAction* reMessage = menu.addAction(QString("��ʱ��Ϣ"));
	QAction* shutDown = menu.addAction(QString("Զ�̹ػ�"));

	connect(disConnect, SIGNAL(triggered()), this, SLOT(onTableWidgetMenuDisConnect()));

	if (m_TableWidget_Online->selectedItems().count() == 0)    //���ûѡ���κ���
	{
		menu.setDisabled(true);
	}
	menu.exec(QCursor::pos());
}
void Remote::onTableWidgetMenuDisConnect()
{
	//�����о�һ�±������в�����������
	int columnCount = m_TableWidget_Online->columnCount();
	int slectedcItemCount = m_TableWidget_Online->selectedItems().count();
	int selectedRowCount = slectedcItemCount / columnCount;
	int currentRow = m_TableWidget_Online->currentRow();         //����ֻ��Ҫ���ѡ���˼��У���һ���Ƕ��٣���Ϊѡ�еĶ�����������
	QString ClientAddressData;
	
	for (int i = 0; i < selectedRowCount; i++)
	{
		ClientAddressData = m_TableWidget_Online->item(currentRow, 0)->text();    //���ÿһ����ĵ�һ��
		ClientAddressData += QString("ǿ�ƶϿ�");
		m_TableWidget_Online->removeRow(currentRow);
		ShowDialogMessage(true, ClientAddressData);
	}

	/*QMessageBox msgBox;
	msgBox.setText(ClientAddressData);
	msgBox.exec();*/

}
void Remote::ShowDialogMessage(bool IsOk, QString Message)
{
	QString v1;
	QString v2;
	QString v3;

	QTime Time = QTime::currentTime();
	v2 = Time.toString();
	if (IsOk)
	{
		v1 = "ִ�гɹ�";
	}
	else
	{
		v1 = "ִ��ʧ��";
	}

	int RowCont;
	RowCont = m_TableWidget_Message->rowCount();
	m_TableWidget_Message->insertRow(RowCont);//����һ��

	m_TableWidget_Message->setItem(RowCont, 0, new QTableWidgetItem(v1));
	m_TableWidget_Message->setItem(RowCont, 1, new QTableWidgetItem(v2));
	m_TableWidget_Message->setItem(RowCont, 2, new QTableWidgetItem(Message));


	if (Message.contains("����") > 0)
	{
		m_ConnectCount++;
	}
	else if (Message.contains("����") > 0)
	{
		m_ConnectCount--;
	}
	else if (Message.contains("�Ͽ�") > 0)
	{
		m_ConnectCount--;
	}
	m_ConnectCount = m_ConnectCount < 0 ? 0 : m_ConnectCount;
	v3 = QString("��%1����������").arg(m_ConnectCount);

	m_StatusBar_Label->setText(v3);
}
void Remote::initMenu()
{
	//todo  ��������ť

	connect(m_Menu_Server_Set, SIGNAL(triggered()), this, SLOT(onCreateServerSetWidget()));
	connect(m_Menu_Server_Exit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_Menu_Server_Add, SIGNAL(triggered()), this, SLOT(onAddTest()));

}
void Remote::onAddTest()
{
	QTableWidgetItem* item1 = new QTableWidgetItem(QString("zhangfei"));
	QTableWidgetItem* item2 = new QTableWidgetItem(QString("guanyu"));
	QTableWidgetItem* item3 = new QTableWidgetItem(QString("liubei"));
	QTableWidgetItem* item4 = new QTableWidgetItem(QString("liubei"));
	QTableWidgetItem* item5 = new QTableWidgetItem(QString("liubei"));
	QTableWidgetItem* item6 = new QTableWidgetItem(QString("liubei"));
	QTableWidgetItem* item7 = new QTableWidgetItem(QString("liubei"));


	int RowCont;
	RowCont = m_TableWidget_Online->rowCount();
	m_TableWidget_Online->insertRow(RowCont);//����һ��

	m_TableWidget_Online->setItem(RowCont, 0, item1);
	m_TableWidget_Online->setItem(RowCont, 1, item2);
	m_TableWidget_Online->setItem(RowCont, 2, item3);
	m_TableWidget_Online->setItem(RowCont, 3, item4);
	m_TableWidget_Online->setItem(RowCont, 4, item5);
	m_TableWidget_Online->setItem(RowCont, 5, item6);
	m_TableWidget_Online->setItem(RowCont, 6, item7);
}
void Remote::insertRowContent(QTableWidget* tablewidget, int row, int column, QTableWidgetItem* tablewidgetItem)
{
	//tablewidget->set
}
void Remote::initToolBar()
{
	m_ToolBar->setIconSize(QSize(48, 48));

	QAction* ToolBarButton_CMD_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_0") ,"�ն˿���");
	QAction* ToolBarButton_PROCESS_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_1"), "���̹���");
	QAction* ToolBarButton_WINDOW_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_2"), "���ڹ���");
	QAction* ToolBarButton_REMOTE_CONTROL = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_3"), "�������");
	QAction* ToolBarButton_FILE_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_4"), "�ļ�����");
	QAction* ToolBarButton_AUDIO_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_5"), "��������");
	QAction* ToolBarButton_CLEAN_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_6"), "ϵͳ����");
	QAction* ToolBarButton_VIDEO_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_7"), "��Ƶ����");
	QAction* ToolBarButton_SERVICE_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_8"), "�������");
	QAction* ToolBarButton_REGISTER_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_9"), "ע������");
	QAction* ToolBarButton_SERVER_MANAGER = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_10"), "���������");
	QAction* ToolBarButton_CLIENT_CLIENT = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_11"), "�ͻ�������");
	QAction* ToolBarButton_SERVER_ABOUT = m_ToolBar->addAction(QIcon(".\\PicRes\\ToolBar_12"), "����");

	//todo  connect
	connect(ToolBarButton_CMD_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonCmdManager()));
	connect(ToolBarButton_PROCESS_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonProcessManager()));
	connect(ToolBarButton_WINDOW_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonWindowManager()));
	connect(ToolBarButton_REMOTE_CONTROL, SIGNAL(triggered()), this, SLOT(onToolButtonRemoteControl()));
	connect(ToolBarButton_FILE_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonFileManager()));
	connect(ToolBarButton_AUDIO_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonAudioManager()));
	connect(ToolBarButton_CLEAN_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonCleanManager()));
	connect(ToolBarButton_VIDEO_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonVideoManager()));
	connect(ToolBarButton_SERVICE_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonServiceManager()));
	connect(ToolBarButton_REGISTER_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonRegisterManager()));
	connect(ToolBarButton_SERVER_MANAGER, SIGNAL(triggered()), this, SLOT(onToolButtonServerManager()));
	connect(ToolBarButton_CLIENT_CLIENT, SIGNAL(triggered()), this, SLOT(onToolButtonServerManager()));
	connect(ToolBarButton_SERVER_ABOUT, SIGNAL(triggered()), this, SLOT(onToolButtonServerAbout()));

}
void Remote::onToolButtonCmdManager()
{
	MMessageBox("111");
}
void Remote::onToolButtonProcessManager()
{

}
void Remote::onToolButtonWindowManager()
{

}
void Remote::onToolButtonRemoteControl()
{

}
void Remote::onToolButtonFileManager()
{

}
void Remote::onToolButtonAudioManager()
{

}
void Remote::onToolButtonCleanManager()
{

}
void Remote::onToolButtonVideoManager()
{

}
void Remote::onToolButtonServiceManager()
{

}
void Remote::onToolButtonRegisterManager()
{

}
void Remote::onToolButtonServerManager()
{

}
void Remote::onToolButtonServerAbout()
{

}
void Remote::initStatusBar()
{
	m_StatusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}")); // ���ò���ʾlabel�ı߿�
	m_StatusBar_Label = new QLabel("Welcome");
	m_StatusBar->addWidget(m_StatusBar_Label);        //��������ô���Label�ķ���������showMessage������˵�ʱstatusbar�ϵ���Ϣ����ʧ
	//m_StatusBar->addPermanentWidget(nullptr);//������Ϣ���� - ���ᱻһ����Ϣ����
	//m_StatusBar->addWidget(nullptr);//������Ϣ���� - �ᱻshowMessage()����Ϣ����
	m_StatusBar->setSizeGripEnabled(false);//ȥ��״̬�����½ǵ�����

}
void Remote::initChildPointer()
{
	m_TableWidget_Online = this->findChild<QTableWidget*>(QString("Server_TableWidget_Online"));
	m_TableWidget_Message = this->findChild<QTableWidget*>(QString("Server_TableWidget_Message"));

	m_Menu_Server_Set = this->findChild<QAction*>(QString("Server_Menu_Set"));
	m_Menu_Server_Exit = this->findChild<QAction*>(QString("Server_Menu_Exit"));
	m_Menu_Server_Add = this->findChild<QAction*>(QString("Server_Menu_Add"));

	m_StatusBar = this->findChild<QStatusBar*>(QString("statusBar"));

	m_ToolBar = this->findChild<QToolBar*>(QString("mainToolBar"));
}
void Remote::MMessageBox(QString message)
{
	QMessageBox a;
	a.setText(message);
	a.exec();
}




void Remote::CreateSystemTrayIconAndMenu()
{
	m_SystemTrayIcon = new QSystemTrayIcon(QIcon(".\\PicRes\\Remote"), this);
	connect(m_SystemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onActiveSystemTrayIcon(QSystemTrayIcon::ActivationReason)));

	m_TrayMenu = new QMenu(this);
	QAction* trayMenuAction_show = new QAction("��ʾ", m_TrayMenu);
	QAction* trayMenuAction_exit = new QAction("�˳�", m_TrayMenu);

	m_TrayMenu->addAction(trayMenuAction_show);
	m_TrayMenu->addAction(trayMenuAction_exit);

	connect(trayMenuAction_show, &QAction::triggered, this, [&]() {
		this->show();
		m_SystemTrayIcon->hide();
		});
	connect(trayMenuAction_exit, &QAction::triggered, this, [&]() {
		isClickFromTrayMenu = true;
		this->close();
		});

	m_SystemTrayIcon->setContextMenu(m_TrayMenu); // �����Ҽ��˵�

	m_SystemTrayIcon->show();
}

void Remote::closeEvent(QCloseEvent* event) 
{
	if (!isClickFromTrayMenu)
	{
		QMessageBox::StandardButton button = QMessageBox::question(this, "��ʾ", "�Ƿ���С�������̣�", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (button == QMessageBox::Yes)
		{
			event->ignore();
			this->hide();
			CreateSystemTrayIconAndMenu();
		}
		else
		{
			event->accept();
		}
	}
	else
	{
		event->accept();
	}
}
void Remote::onActiveSystemTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
		break;
	case QSystemTrayIcon::DoubleClick:
		this->show();
		m_SystemTrayIcon->hide();
	}
}