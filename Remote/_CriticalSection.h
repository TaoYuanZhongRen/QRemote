#pragma once
#include "Windows.h"
class _CriticalSection
{
public:

	_CriticalSection(CRITICAL_SECTION* CriticalSection);
	~_CriticalSection();


public:
	void Lock();
	void Unlock();

private:
	CRITICAL_SECTION* m_CriticalSection;
};

