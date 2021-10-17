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

static LPTSTR szGPL =
	_T("MFILEMON - print to file with automatic filename assignment\n")
	_T("Copyright (C) 2007-2021 Monti Lorenzo\n")
	_T("\n")
	_T("This program is free software; you can redistribute it and/or\n")
	_T("modify it under the terms of the GNU General Public License\n")
	_T("as published by the Free Software Foundation; either version 3\n")
	_T("of the License, or (at your option) any later version.\n")
	_T("\n")
	_T("This program is distributed in the hope that it will be useful,\n")
	_T("but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
	_T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
	_T("GNU General Public License for more details.\n")
	_T("\n")
	_T("You should have received a copy of the GNU General Public License\n")
	_T("along with this program; if not, write to the Free Software\n")
	_T("Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.");

static LPTSTR szUsage =
	_T("**************************************\n")
	_T("Usage: regmon [-r | -d]\n")
	_T("       -r: register monitor\n")
	_T("       -d: deregister monitor\n")
	_T("       -l: list registered monitors\n")
	_T("**************************************");

static BOOL ListRegisteredMonitors()
{
	DWORD pcbNeeded = 0;
	DWORD pcReturned = 0;

	EnumMonitors(NULL, 2, (LPBYTE)NULL, 0, &pcbNeeded, &pcReturned);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		//_tprintf(_T("%s\n"), "ERROR_INSUFFICIENT_BUFFER");
		return FALSE;
	}

	LPBYTE pPorts = (LPBYTE)malloc(sizeof(BYTE)*pcbNeeded);
	memset(pPorts, 0, sizeof(BYTE)*pcbNeeded);
	BOOL result = EnumMonitors(NULL, 2, pPorts, pcbNeeded, &pcbNeeded, &pcReturned);
	if (!result)
	{
		DWORD dwErr = GetLastError();
		free(pPorts);
		SetLastError(dwErr);
		//_tprintf(_T("%s\n"), "Could not get monitors.");
		return FALSE;
	}

	MONITOR_INFO_2 *pinfo = (MONITOR_INFO_2*)pPorts;

	for (DWORD i = 0; i < pcReturned; i++)
	{
		_tprintf(_T("%s\n"), pinfo[i].pName);
	}

	free(pPorts);
	
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_tprintf(_T("%s\n\n"), szGPL);

	_tprintf(_T("%s\n\n"), szUsage);

	MONITOR_INFO_2 minfo = { 0 };
	LPTSTR szAction = NULL;
	int ret;

	minfo.pName = _T("Multi File Port Monitor");
	minfo.pEnvironment = NULL;
	minfo.pDLLName = _T("mfilemon.dll");

	if (argc > 1 && _tcsicmp(argv[1], _T("-r")) == 0)
	{
		szAction = _T("AddMonitor");
		ret = AddMonitor(NULL, 2, (LPBYTE)&minfo);
	}
	else if (argc > 1 && _tcsicmp(argv[1], _T("-d")) == 0)
	{
		szAction = _T("DeleteMonitor");
		ret = DeleteMonitor(NULL, NULL, minfo.pName);
	}
	else if (argc > 1 && _tcsicmp(argv[1], _T("-l")) == 0)
	{
		szAction = _T("ListMonitors");
		//_tprintf(_T("%s\n"), L"Start list monitors...");
		ret = ListRegisteredMonitors();
		//_tprintf(_T("%s\n\n"), L"End list monitors...");
	}
	else
	{
		return 1;
	}

	if (ret == FALSE)
	{
		DWORD dwErr = GetLastError();
		LPVOID pMsgBuf = NULL;
		if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf, 0, NULL))
		{
			_tprintf(_T("The following error occurred:\n0x%08x\n%s\n"),
				dwErr, static_cast<LPCTSTR>(pMsgBuf));
			LocalFree(pMsgBuf);
		}
	}

	_tprintf(_T("%s %s\n"), szAction, ret == FALSE ? _T("FAILED!!!") : _T("SUCCEEDED!!!"));

	return (ret == FALSE);
}
