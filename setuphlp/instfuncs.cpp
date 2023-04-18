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

#include "stdafx.h"

static const LPCWSTR pMonitorName = L"Auto Multi File Port Monitor";

BOOL __stdcall RegisterMonitor()
{
	MONITOR_INFO_2W minfo = { 0 };

	minfo.pName = _wcsdup(pMonitorName);
	minfo.pEnvironment = NULL;
	minfo.pDLLName = _wcsdup(L"amfilemon.dll");

	BOOL bRet = AddMonitorW(NULL, 2, (LPBYTE)&minfo);

	if (minfo.pName)
		free(minfo.pName);

	if (minfo.pDLLName)
		free(minfo.pDLLName);

	return bRet;
}

BOOL __stdcall UnregisterMonitor()
{
	LPWSTR strName = _wcsdup(pMonitorName);

	BOOL bRet = DeleteMonitorW(NULL, NULL, strName);

	if (strName)
		free(strName);

	return bRet;
}

BOOL __stdcall IsMonitorRegistered()
{
	DWORD pcbNeeded = 0;
	DWORD pcReturned = 0;

	EnumMonitorsW(NULL, 2, NULL, 0, &pcbNeeded, &pcReturned);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return TRUE;

	LPBYTE pPorts = new BYTE[pcbNeeded];
	memset(pPorts, 0, sizeof(BYTE)*pcbNeeded);
	BOOL result = EnumMonitorsW(NULL, 2, pPorts, pcbNeeded, &pcbNeeded, &pcReturned);
	if (!result)
	{
		delete[] pPorts;
		return TRUE;
	}

	MONITOR_INFO_2W *pinfo = reinterpret_cast<MONITOR_INFO_2W*>(pPorts);

	for (; pcReturned-- > 0; pinfo++)
	{
		if (wcscmp(pMonitorName, pinfo->pName) == 0)
		{
			delete[] pPorts;
			return TRUE;
		}
	}

	delete[] pPorts;

	return FALSE;
}