/*
MFILEMON - print to file with automatic filename assignment
Copyright (C) 2007-2021 Monti Lorenzo

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

static const LPWSTR pMonitorName = L"Multi File Port Monitor";

BOOL __stdcall RegisterMonitor()
{
	MONITOR_INFO_2W minfo = { 0 };

	minfo.pName = pMonitorName;
	minfo.pEnvironment = NULL;
	minfo.pDLLName = L"mfilemon.dll";

	return AddMonitorW(NULL, 2, (LPBYTE)&minfo);
}

BOOL __stdcall UnregisterMonitor()
{
	return DeleteMonitorW(NULL, NULL, pMonitorName);
}

BOOL __stdcall IsMonitorRegistered()
{
	DWORD pcbNeeded = 0;
	DWORD pcReturned = 0;

	EnumMonitorsW(NULL, 2, (LPBYTE)NULL, 0, &pcbNeeded, &pcReturned);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return TRUE;

	LPBYTE pPorts = (LPBYTE)malloc(sizeof(BYTE)*pcbNeeded);
	memset(pPorts, 0, sizeof(BYTE)*pcbNeeded);
	BOOL result = EnumMonitorsW(NULL, 2, pPorts, pcbNeeded, &pcbNeeded, &pcReturned);
	if (!result)
	{
		free(pPorts);
		return TRUE;
	}

	MONITOR_INFO_2W *pinfo = (MONITOR_INFO_2W*)pPorts;

	for (DWORD i = 0; i < pcReturned; i++)
	{
		if (wcscmp(pMonitorName, pinfo[i].pName) == 0)
		{
			free(pPorts);
			return TRUE;
		}
	}

	free(pPorts);
	
	return FALSE;
}