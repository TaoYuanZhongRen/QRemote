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
	GetModuleFileNameA(NULL, FileFullPath, MAX_PATH);     //��õ�ǰ��ִ���ļ��ľ���·��

	CHAR* v1 = NULL;
	v1 = strstr(FileFullPath, ".");
	if (v1 != NULL)
	{
		*v1 = '\0';
		strcat(FileFullPath, ".ini");    //�õ������ļ�·��
	}

	m_FileFullPath = FileFullPath;
	//����һ��ini�ļ�
	QSettings configini(m_FileFullPath.c_str() , QSettings::IniFormat);
	//�����ļ����룬�����ļ���ʹ������ʱ�����Ǳ���ģ���������
	//configini.setIniCodec(QTextCodec::codecForName("UTF-8"));

	

	if (QFile::exists(m_FileFullPath.c_str()))
	{	// �ļ����ڣ�����������
		return false;
	}
	else 
	{
		// �ļ������ڣ�д����������������ļ�
		configini.setValue("Settings/ListenPort", "2356");
		configini.setValue("Settings/MaxConnection", "10");
		// setValueֻ�ǰ�������д���˻���������Ҫд���ļ�������ִ��ͬ��
		// ��ͬ�����޷�д���ļ����޷������ļ�
		configini.sync();
	}





	//HANDLE FileHandle = CreateFileA(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,   //��ռ
	//	NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN, NULL);      //ͬ���첽

	//if (FileHandle == INVALID_HANDLE_VALUE)
	//{
	//	return false;
	//}
	//m_FileFullPath = FileFullPath;
	//LARGE_INTEGER FileSize;               //����һ���ṹ��
	//GetFileSizeEx(FileHandle, &FileSize);    //����������Զ�ȡ����4G���ļ���С
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
	configini.sync();//д�������ļ�
	configini.endGroup();
}
