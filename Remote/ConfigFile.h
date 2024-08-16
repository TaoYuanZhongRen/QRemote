#pragma once
#include<qsettings.h>
#include <qfile.h>

class ConfigFile
{
public:
	ConfigFile();
	~ConfigFile();

	bool initConfigFile();

	unsigned int GetInt(QString MainKey, QString SubKey);
	void SetInt(QString MainKey, QString SubKey, unsigned int BufferData);

private:
	std::string m_FileFullPath;       //配置文件的绝对路径
};

