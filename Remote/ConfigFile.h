#pragma once

#include<Windows.h>
#include<qsettings.h>
#include <qfile.h>

class ConfigFile
{
public:
	ConfigFile();
	~ConfigFile();

	bool initConfigFile();

	int GetInt(QString MainKey, QString SubKey);
	void SetInt(QString MainKey, QString SubKey, int BufferData);

private:
	std::string m_FileFullPath;       //配置文件的绝对路径
};

