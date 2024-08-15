#include "ConfigFile.h"


ConfigFile::ConfigFile()
{
	initConfigFile();
}

ConfigFile::~ConfigFile()
{
}

bool ConfigFile::initConfigFile()
{
	CHAR FileFullPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, FileFullPath, MAX_PATH);     //获得当前可执行文件的绝对路径

	CHAR* v1 = NULL;
	v1 = strstr(FileFullPath, ".");
	if (v1 != NULL)
	{
		*v1 = '\0';
		strcat(FileFullPath, ".ini");    //拿到配置文件路径
	}

	m_FileFullPath = FileFullPath;
	//创建一个ini文件
	QSettings configini(m_FileFullPath.c_str() , QSettings::IniFormat);
	//设置文件编码，配置文件中使用中文时，这是必须的，否则乱码
	//configini.setIniCodec(QTextCodec::codecForName("UTF-8"));

	

	if (QFile::exists(m_FileFullPath.c_str()))
	{	// 文件存在，读出配置项
		return false;
	}
	else 
	{
		// 文件不存在，写入配置项，生成配置文件
		configini.setValue("Settings/ListenPort", "2356");
		configini.setValue("Settings/MaxConnection", "10");
		// setValue只是把配置项写入了缓冲区，若要写入文件，还需执行同步
		// 不同步，无法写入文件，无法生成文件
		configini.sync();
	}





	//HANDLE FileHandle = CreateFileA(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,   //独占
	//	NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN, NULL);      //同步异步

	//if (FileHandle == INVALID_HANDLE_VALUE)
	//{
	//	return false;
	//}
	//m_FileFullPath = FileFullPath;
	//LARGE_INTEGER FileSize;               //定义一个结构体
	//GetFileSizeEx(FileHandle, &FileSize);    //这个函数可以读取大于4G的文件大小
	//if (FileSize.QuadPart > 0)
	//{
	//	CloseHandle(FileHandle);
	//	return false;
	//}
	// CloseHandle(FileHandle);
	//WritePrivateProfileStringA("Settings", "ListenPort", "2356", m_FileFullPath.c_str());
	//auto a = GetLastError();
	//WritePrivateProfileStringA("Settings", "MaxConnection", "10", m_FileFullPath.c_str());
	return true;
}



int ConfigFile::GetInt(QString MainKey,QString SubKey)
{
	QSettings configini(m_FileFullPath.c_str(), QSettings::IniFormat);
	configini.beginGroup(MainKey);
	int value = configini.value(SubKey).toInt();
	configini.endGroup();
	return value;
}
void ConfigFile::SetInt(QString MainKey, QString SubKey, int BufferData)
{
	QSettings configini(m_FileFullPath.c_str(), QSettings::IniFormat);
	configini.beginGroup(MainKey);
	configini.setValue(SubKey, BufferData);
	configini.sync();//写入配置文件
	configini.endGroup();
}
