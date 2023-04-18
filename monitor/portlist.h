/*
MFILEMON - print to file with automatic filename assignment
Copyright (C) 2007-2023 Lorenzo Monti

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#include "port.h"
#include "..\common\config.h"

class CPortList
{
private:
	typedef struct tagPORTREC
	{
		tagPORTREC()
		{
			m_pPort = NULL;
			m_pNext = NULL;
		}
		~tagPORTREC()
		{
			if (m_pPort)
				delete m_pPort;
		}
		CPort* m_pPort;
		tagPORTREC* m_pNext;
	} PORTREC, *LPPORTREC;

private:
	static LPCWSTR szOutputPathKey;
	static LPCWSTR szFilePatternKey;
	static LPCWSTR szOverwriteKey;
	static LPCWSTR szUserCommandPatternKey;
	static LPCWSTR szExecPathKey;
	static LPCWSTR szWaitTerminationKey;
	static LPCWSTR szWaitTimeoutKey;
	static LPCWSTR szPipeDataKey;
	static LPCWSTR szLogLevelKey;
	static LPCWSTR szUserKey;
	static LPCWSTR szDomainKey;
	static LPCWSTR szPasswordKey;
	static LPCWSTR szHideProcessKey;
	LPPORTREC m_pFirstPortRec;
	WCHAR m_szMonitorName[MAX_PATH + 1];
	WCHAR m_szPortDesc[MAX_PATH + 1];
	CRITICAL_SECTION m_CSPortList;

public:
	CPortList(LPCWSTR szPortMonitorName, LPCWSTR szPortDesc);
	virtual ~CPortList();

public:
	void AddMfmPort(LPPORTCONFIG pConfig);
	void AddMfmPort(CPort* pNewPort);
	void DeletePort(CPort* pPortToDelete);
	CPort* FindPort(LPCWSTR szPortName);
	BOOL EnumPorts(HANDLE hMonitor, LPCWSTR pName, DWORD Level, LPBYTE pPorts,
		DWORD cbBuf, LPDWORD pcbNeeded, LPDWORD pcReturned);
	void LoadFromRegistry();
	void SaveToRegistry();
	LPCRITICAL_SECTION GetCriticalSection() { return &m_CSPortList; }

private:
	DWORD GetPortSize(LPCWSTR szPortName, DWORD dwLevel);
	LPBYTE CopyPortToBuffer(CPort* pPort, DWORD dwLevel, LPBYTE pStart, LPBYTE pEnd);
	void RemoveFromRegistry(CPort* pPort);
};

extern CPortList* g_pPortList;
