/*
MFILEMON - print to file with automatic filename assignment
Copyright (C) 2007-2021 Lorenzo Monti

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
#include "log.h"
#include "port.h"
#include <string.h>
#include <stdarg.h>
//---------------------------------------------------------------------------

static const unsigned short int BOM = 0xFEFF;

CMfmLog* g_pLog = NULL;
//---------------------------------------------------------------------------

#define CHECK_LEVEL(lev) do { if (m_hLogFile == INVALID_HANDLE_VALUE || m_nLogLevel < lev) return; } while (0)
//---------------------------------------------------------------------------

CMfmLog::CMfmLog()
: m_nLogLevel(LOGLEVEL_NONE), m_bFlushNeeded(FALSE)
{
	m_szBuffer[0] = L'\0';

	m_hStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThread = CreateThread(NULL, 0, FlushThread, this, CREATE_SUSPENDED, NULL);

	CreateLogFile();

	InitializeCriticalSection(&m_CSLog);

	if (m_hThread)
		ResumeThread(m_hThread);
}
//---------------------------------------------------------------------------

CMfmLog::~CMfmLog()
{
	SetEvent(m_hStop);

	if (m_hThread)
	{
		if (WaitForSingleObject(m_hThread, 10000) != WAIT_OBJECT_0)
			TerminateThread(m_hThread, 255);

		CloseHandle(m_hThread);
	}

	CloseHandle(m_hStop);

	if (m_hLogFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hLogFile);

	DeleteCriticalSection(&m_CSLog);
}
//---------------------------------------------------------------------------

DWORD WINAPI CMfmLog::FlushThread(LPVOID pParam)
{
	CMfmLog* pLog = static_cast<CMfmLog*>(pParam);

	while (TRUE)
	{
		switch (WaitForSingleObject(pLog->m_hStop, 10000))
		{
		case WAIT_OBJECT_0:
			return 0;
			break;

		case WAIT_TIMEOUT:
			EnterCriticalSection(&pLog->m_CSLog);

			if (pLog->m_bFlushNeeded && pLog->m_hLogFile != INVALID_HANDLE_VALUE)
				FlushFileBuffers(pLog->m_hLogFile);

			LeaveCriticalSection(&pLog->m_CSLog);
			break;
		}
	}
}
//---------------------------------------------------------------------------

BOOL CMfmLog::CreateLogFile()
{
	WCHAR szPath[MAX_PATH + 1];

	GetSystemDirectoryW(szPath, LENGTHOF(szPath));
	wcscat_s(szPath, LENGTHOF(szPath), L"\\mfilemon.log");

	m_hLogFile = CreateFileW(szPath, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);

	if (m_hLogFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		SetFilePointer(m_hLogFile, 0, NULL, FILE_END);
	}
	else
	{
		DWORD wri;
		WriteFile(m_hLogFile, &BOM, sizeof(BOM), &wri, NULL);
		m_bFlushNeeded = TRUE;
	}

	return TRUE;
}
//---------------------------------------------------------------------------

void CMfmLog::RotateLogs()
{
	if (m_hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hLogFile);
		m_hLogFile = INVALID_HANDLE_VALUE;
	}

	for (int n = 9; n >= 0; n--)
	{
		WCHAR szOldFname[32];
		WCHAR szNewFname[32];

		WCHAR szOldPath[MAX_PATH + 1];
		WCHAR szNewPath[MAX_PATH + 1];

		GetSystemDirectoryW(szOldPath, LENGTHOF(szOldPath));

		if (n == 0)
		{
			wcscat_s(szOldPath, LENGTHOF(szOldPath), L"\\mfilemon.log");
		}
		else
		{
			swprintf_s(szOldFname, LENGTHOF(szOldFname), L"\\mfilemon.%i.log", n);
			wcscat_s(szOldPath, LENGTHOF(szOldPath), szOldFname);
		}

		if (n == 9)
		{
			DeleteFileW(szOldPath);
		}
		else
		{
			GetSystemDirectoryW(szNewPath, LENGTHOF(szNewPath));

			swprintf_s(szNewFname, LENGTHOF(szNewFname), L"\\mfilemon.%i.log", n + 1);
			wcscat_s(szNewPath, LENGTHOF(szNewPath), szNewFname);

			MoveFileW(szOldPath, szNewPath);
		}
	}
}
//---------------------------------------------------------------------------

void CMfmLog::SetLogLevel(DWORD nLevel)
{
	if (nLevel < LOGLEVEL_MIN)
		nLevel = LOGLEVEL_MIN;
	else if (nLevel > LOGLEVEL_MAX)
		nLevel = LOGLEVEL_MAX;

	m_nLogLevel = nLevel;
}
//---------------------------------------------------------------------------

void CMfmLog::Always(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_NONE);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"NONE", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Debug(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_DEBUG);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"DEBUG", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Info(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"INFO", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Done(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"DONE", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Warn(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_WARNINGS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"WARN", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Error(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"ERROR", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Critical(LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(szFormat, L"CRITICAL", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Always(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_NONE);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"NONE", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Debug(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_DEBUG);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"DEBUG", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Info(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"INFO", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Done(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"DONE", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Warn(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_WARNINGS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"WARN", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Error(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"ERROR", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::Critical(CPort* pPort, LPCWSTR szFormat, ...)
{
	CHECK_LEVEL(LOGLEVEL_ERRORS);

	va_list args;

	va_start(args, szFormat);
	LogArgs(pPort, szFormat, L"CRITICAL", args);
	va_end(args);
}
//---------------------------------------------------------------------------

void CMfmLog::LogArgs(CPort* pPort, LPCWSTR szFormat, LPCWSTR szType, va_list args)
{
	WCHAR* szBuf1 = new WCHAR[MAXLOGLINE];
	WCHAR* szBuf2 = new WCHAR[MAXLOGLINE];

	vswprintf_s(szBuf1, MAXLOGLINE, szFormat, args);
	swprintf_s(szBuf2, MAXLOGLINE, L"%s: %s", pPort->PortName(), szBuf1);

	Log(szType, szBuf2);

	delete[] szBuf1;
	delete[] szBuf2;
}
//---------------------------------------------------------------------------

void CMfmLog::LogArgs(LPCWSTR szFormat, LPCWSTR szType, va_list args)
{
	WCHAR* szMessage = new WCHAR[MAXLOGLINE];

	vswprintf_s(szMessage, MAXLOGLINE, szFormat, args);
	Log(szType, szMessage);

	delete[] szMessage;
}
//---------------------------------------------------------------------------

void CMfmLog::Log(LPCWSTR szType, LPCWSTR szMessage)
{
	SYSTEMTIME st;
	DWORD wri;
	const DWORD MAXLOGSIZE = 10 * 1024 * 1024;

	GetLocalTime(&st);

	int len = swprintf_s(m_szBuffer, LENGTHOF(m_szBuffer),
		L"%02i-%02i-%04i %02i:%02i:%02i.%03i  [%s] %s\r\n",
		st.wDay, st.wMonth, st.wYear,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		szType,
		szMessage
	);

	if (len > 0)
	{
		EnterCriticalSection(&m_CSLog);

		DWORD dwSize, dwSizeHigh;
		dwSize = GetFileSize(m_hLogFile, &dwSizeHigh);
		if (dwSize >= MAXLOGSIZE || dwSizeHigh > 0)
		{
			RotateLogs();
			if (!CreateLogFile())
				goto LExit;
		}

		WriteFile(m_hLogFile, m_szBuffer, len * sizeof(WCHAR), &wri, NULL);
		m_bFlushNeeded = TRUE;

	LExit:
		LeaveCriticalSection(&m_CSLog);
	}
}
//---------------------------------------------------------------------------
