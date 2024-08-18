#include "_CriticalSection.h"

_CriticalSection::_CriticalSection(CRITICAL_SECTION* CriticalSection)
	: m_CriticalSection(CriticalSection)
{
	Lock();
}

_CriticalSection::~_CriticalSection()
{
	Unlock();
	delete m_CriticalSection;
	m_CriticalSection = nullptr;
}

void _CriticalSection::Lock()
{
	EnterCriticalSection(m_CriticalSection);
}

void _CriticalSection::Unlock()
{
	LeaveCriticalSection(m_CriticalSection);
}
