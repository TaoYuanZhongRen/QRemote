#pragma once

#include "ui_Remote.h"
#include "WidgetServerSet.h"
#include <map>
#include <qdatetime.h>
#include <qdebug.h>
#include <QDesktopServices> //桌面事件类
#include <qevent.h>
#include <QMessageBox>
#include <qstandarditemmodel.h>
#include <QSystemTrayIcon>  //t托盘类
#include <qtablewidget.h>
#include <qtextcodec.h>.
#include <QtWidgets/QMainWindow>
#include <vector>

class Remote : public QMainWindow
{
    Q_OBJECT

public:
    Remote(QWidget *parent = nullptr);
    ~Remote();

    void initMenu();

    void initToolBar();

    void initStatusBar();

    void initChildPointer();

    void MessageBox(QString message);
    
    void initListWidget();

    void initTableHeader();



public slots:
    void onAddTest();
    void onCreateServerSetWidget();
    void onTableWidgetSelectChanged();
    void onCreateTableWidgetMenu();

    void onTableWidgetMenuDisConnect();

    
    void onToolButtonCmdManager();
    void onToolButtonProcessManager();
    void onToolButtonWindowManager();
    void onToolButtonRemoteControl();
    void onToolButtonFileManager();
    void onToolButtonAudioManager();
    void onToolButtonCleanManager();
    void onToolButtonVideoManager();
    void onToolButtonServiceManager();
    void onToolButtonRegisterManager();
    void onToolButtonServerManager();
    void onToolButtonServerAbout();

    void onActiveSystemTrayIcon(QSystemTrayIcon::ActivationReason reason);

private:
    void insertRowContent(QTableWidget* tablewidget, int row, int column, QTableWidgetItem* tablewidgetItem);
    void ShowDialogMessage(bool IsOk, QString Message);

protected:
    void closeEvent(QCloseEvent* event) override;
    void CreateSystemTrayIconAndMenu();

private:
    Ui::RemoteClass ui;
    QTableWidget* m_TableWidget_Online;
    QTableWidget* m_TableWidget_Message;

    QMenu* m_Menu_Server;
    
    QAction* m_Menu_Server_Set;
    QAction* m_Menu_Server_Exit;
    QAction* m_Menu_Server_Add;

    QStatusBar* m_StatusBar;
    QLabel* m_StatusBar_Label;

    QToolBar* m_ToolBar;

    WidgetServerSet* m_Widget_Server_Set;


    QSystemTrayIcon* m_SystemTrayIcon;
    QMenu* m_TrayMenu;
    bool isClickFromTrayMenu = false;
};
