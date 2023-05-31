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
#include "port.h"
#include "log.h"
#include "..\common\autoclean.h"
#include "..\common\defs.h"
#include "..\common\monutils.h"
#include <VersionHelpers.h>

//-------------------------------------------------------------------------------------
static BOOL EnablePrivilege(
	HANDLE hToken,                      // access token handle
	LPCWSTR lpszPrivilege,              // name of privilege to enable/disable
	BOOL bEnablePrivilege,              // to enable or disable privilege
	PTOKEN_PRIVILEGES PreviousState		// previous state
)
{
	TOKEN_PRIVILEGES tp = { 0 };
	LUID luid;

	if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid))
	{
		return FALSE; 
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	DWORD Size = sizeof(TOKEN_PRIVILEGES);
	// Enable the privilege or disable all privileges.
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
		PreviousState, &Size))
	{ 
		return FALSE; 
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	} 

	return TRUE;
}

//-------------------------------------------------------------------------------------
static BOOL GetPrimaryToken(LPWSTR lpszUsername, LPWSTR lpszDomain, LPWSTR lpszPassword,
	PHANDLE phToken, BOOL *bRestrictedToken)
{
	DWORD dwLength;
	*bRestrictedToken = FALSE;
	BOOL bIsWindowsVistaOrLater = IsWindowsVistaOrGreater();

	if (!bIsWindowsVistaOrLater)
	{
		return LogonUserW(lpszUsername, lpszDomain, lpszPassword, LOGON32_LOGON_INTERACTIVE,
			LOGON32_PROVIDER_DEFAULT, phToken);
	}
	else
	{
		/*
		* Assume activated User Account Control.
		* To retrieve user primary token, it is better to avoid LOGON32_LOGON_INTERACTIVE logon type,
		* for LogonUser, otherwise we'll get the filtered token with missing privileges.
		*
		* Thus I'll try first to logon as batch or service.
		* If it works, I've got user token with his highest privileges,
		* done.
		*
		* Otherwise,
		* - logon with interactive logon type and get possibly filtered token.
		* - Try to enable TCB privilege.
		* - Retrieve linked token via GetTokenInformation().
		* If an error or linked token is NULL, then either UAC is not active or we
		* got a standard user, return logon token, done.
		* If linked token is not NULL and TCB privilege was enabled,
		* return linked token (it is primary due to TCB held).
		*
		* If linked token is not NULL and TCB was NOT enabled,
		* returned token is restricted (filtered). This should not happen often,
		* because there is a fair number of conditions that must hold true,
		* 1) User is an Admin
		* 2) User has no batch neither service logon privileges
		* 3) Caller has no TCB privilege
		* 4) UAC is enabled.
		*/

		int i;
		HANDLE hMyToken;
		HANDLE hLinkedToken;
		DWORD logonType = LOGON32_LOGON_BATCH;
		BOOL bSuccess = FALSE;
		DWORD dwLastError;
		BOOL bGotTcbPriv;
		TOKEN_PRIVILEGES TcbPrevState;
		DWORD allLogonTypes [] = {
			LOGON32_LOGON_BATCH,
			LOGON32_LOGON_SERVICE,
			LOGON32_LOGON_INTERACTIVE /* intentionally put last, as most restrictive logon */
		};

		/* Try all logon types */
		for (i = 0; i < LENGTHOF(allLogonTypes); i++)
		{
			logonType = allLogonTypes[i];
			bSuccess = LogonUserW(lpszUsername, lpszDomain, lpszPassword, logonType,
				LOGON32_PROVIDER_DEFAULT, phToken);

			if (bSuccess)
				break;

			dwLastError = GetLastError();

			if (dwLastError != ERROR_LOGON_TYPE_NOT_GRANTED &&
				dwLastError != ERROR_LOGON_NOT_GRANTED)
			{
				return FALSE;
			}
		}

		if (!bSuccess)
		{
			/* User could not logon with any logon type? */
			return FALSE;
		}

		if (logonType != LOGON32_LOGON_INTERACTIVE)
		{
			/* Non-interactive logon, no UAC, no token restrictions */
			return TRUE;
		}

		/* Try to get the highest privileged token */
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hMyToken))
		{
			return FALSE;
		}

		/* Enable TCB temporarily to get primary "linked" token .
		* Without TCB GetTokenInformation() would return
		* Identity token, unusable for CreateProcessAsUser()
		*/

		bGotTcbPriv = EnablePrivilege(hMyToken, SE_TCB_NAME, TRUE, &TcbPrevState);
		bSuccess = GetTokenInformation(*phToken, TokenLinkedToken, (VOID*)&hLinkedToken,
			sizeof(HANDLE), &dwLength);

		if (bGotTcbPriv)
		{
			/* Reset TCB privilege, if was set previously */
			AdjustTokenPrivileges(hMyToken, FALSE, &TcbPrevState, sizeof(TcbPrevState), NULL, NULL);
		}

		CloseHandle(hMyToken);

		if (!bSuccess)
		{
			if ((dwLastError = GetLastError()) == ERROR_NO_SUCH_LOGON_SESSION)
			{
				/* Can happend if we have standard user/UAC switched off */
				SetLastError(ERROR_SUCCESS);
				hLinkedToken = NULL;
			}
			else
			{
				return FALSE;
			}
		}

		if (!hLinkedToken)
		{
			/* No UAC or standard user */
			return TRUE;
		}

		if (!bGotTcbPriv)
		{
			/* Could not enable TCB , *phToken is restricted */
			*bRestrictedToken = TRUE;
			return TRUE;
		}

		CloseHandle(*phToken);

		/* primary linked token */
		*phToken = hLinkedToken;
		return TRUE;
	}
}

//-------------------------------------------------------------------------------------
CPort::CPort()
{
	Initialize();
}

//-------------------------------------------------------------------------------------
CPort::CPort(LPCWSTR szPortName)
{
	Initialize(szPortName);
}

//-------------------------------------------------------------------------------------
CPort::CPort(LPPORTCONFIG pPortConfig)
{
	Initialize(pPortConfig);
}

//-------------------------------------------------------------------------------------
void CPort::Initialize()
{
	*m_szPortName = L'\0';
	*m_szOutputPath = L'\0';
	m_szPrinterName = NULL;
	m_cchPrinterName = 0;
	m_pPattern = NULL;
	m_bOverwrite = FALSE;
	m_pUserCommand = NULL;
	*m_szExecPath = L'\0';
	m_bWaitTermination = FALSE;
	m_dwWaitTimeout = 0;
	m_bPipeData = FALSE;
	m_bHideProcess = TRUE;
	*m_szFileName = L'\0';
	m_hFile = INVALID_HANDLE_VALUE;
	m_hWriteThread = NULL;
	m_hWorkEvt = NULL;
	m_hDoneEvt = NULL;
	m_nJobId = 0;
	m_pJobInfo2 = NULL;
	m_cbJobInfo2 = 0;
	m_bPipeActive = FALSE;
	InitializeCriticalSection(&m_threadData.csBuffer);
	*m_szUser = L'\0';
	wcscpy_s(m_szDomain, LENGTHOF(m_szDomain), L".");
	*m_szPassword = L'\0';
	m_hToken = NULL;
	m_bRestrictedToken = FALSE;
	m_bLogonInvalidated = TRUE;
}

//-------------------------------------------------------------------------------------
void CPort::Initialize(LPCWSTR szPortName)
{
	Initialize();
	wcscpy_s(m_szPortName, LENGTHOF(m_szPortName), szPortName);
}

//-------------------------------------------------------------------------------------
void CPort::Initialize(LPPORTCONFIG pConfig)
{
	Initialize(pConfig->szPortName);
	wcscpy_s(m_szOutputPath, LENGTHOF(m_szOutputPath), pConfig->szOutputPath);
	SetFilePatternString(pConfig->szFilePattern);
	m_bOverwrite = pConfig->bOverwrite;
	SetUserCommandString(pConfig->szUserCommandPattern);
	wcscpy_s(m_szExecPath, LENGTHOF(m_szExecPath), pConfig->szExecPath);
	m_bWaitTermination = pConfig->bWaitTermination;
	m_dwWaitTimeout = pConfig->dwWaitTimeout;
	if (m_dwWaitTimeout > 4294967)
		m_dwWaitTimeout = 4294967;
	m_bPipeData = pConfig->bPipeData;
	m_bHideProcess = pConfig->bHideProcess;
	wcscpy_s(m_szUser, LENGTHOF(m_szUser), pConfig->szUser);
	Trim(m_szUser);
	wcscpy_s(m_szDomain, LENGTHOF(m_szDomain), pConfig->szDomain);
	Trim(m_szDomain);
	if (!*m_szDomain)
		wcscpy_s(m_szDomain, LENGTHOF(m_szDomain), L".");
	wcscpy_s(m_szPassword, LENGTHOF(m_szPassword), pConfig->szPassword);

	g_pLog->Info(L"Initializing port %s", m_szPortName);
	g_pLog->Info(L" Output path:         %s", m_szOutputPath);
	g_pLog->Info(L" File pattern:        %s", pConfig->szFilePattern);
	g_pLog->Info(L" Overwrite:           %s", (m_bOverwrite ? szTrue : szFalse));
	g_pLog->Info(L" User command:        %s", pConfig->szUserCommandPattern);
	g_pLog->Info(L" Execute from:        %s", m_szExecPath);
	g_pLog->Info(L" Wait termination:    %s", (m_bWaitTermination ? szTrue : szFalse));
	g_pLog->Info(L" Wait timeout:        %u", m_dwWaitTimeout);
	g_pLog->Info(L" Use pipe:            %s", (m_bPipeData ? szTrue : szFalse));
	if (wcschr(m_szUser, L'@') != NULL)
		g_pLog->Info(L" Run as:              %s", m_szUser);
	else
		g_pLog->Info(L" Run as:              %s\\%s", m_szDomain, m_szUser);
}

//-------------------------------------------------------------------------------------
CPort::~CPort()
{
	if (m_pPattern)
		delete m_pPattern;

	if (m_pUserCommand)
		delete m_pUserCommand;

	if (m_szPrinterName)
		delete[] m_szPrinterName;

	if (m_pJobInfo2)
		delete[] m_pJobInfo2;

	if (m_hWriteThread)
	{
		EnterCriticalSection(&m_threadData.csBuffer);
		m_threadData.lpBuffer = NULL;
		LeaveCriticalSection(&m_threadData.csBuffer);
		SetEvent(m_hWorkEvt);
		WaitForSingleObject(m_hDoneEvt, INFINITE);
		CloseHandle(m_hWriteThread);
		m_hWriteThread = NULL;
	}

	if (m_hWorkEvt)
		CloseHandle(m_hWorkEvt);

	if (m_hDoneEvt)
		CloseHandle(m_hDoneEvt);

	DeleteCriticalSection(&m_threadData.csBuffer);
}

//-------------------------------------------------------------------------------------
void CPort::SetFilePatternString(LPCWSTR szPattern)
{
	if (m_pPattern)
		delete m_pPattern;

	m_pPattern = new CPattern(szPattern, this, FALSE);
}

//-------------------------------------------------------------------------------------
void CPort::SetUserCommandString(LPCWSTR szPattern)
{
	if (m_pUserCommand)
		delete m_pUserCommand;

	m_pUserCommand = new CPattern(szPattern, this, TRUE);
}

//-------------------------------------------------------------------------------------
LPCWSTR CPort::FilePattern() const
{
	if (m_pPattern)
		return m_pPattern->PatternString();
	else
		return CPattern::szDefaultFilePattern;
}

//-------------------------------------------------------------------------------------
LPCWSTR CPort::UserCommandPattern() const
{
	if (m_pUserCommand)
		return m_pUserCommand->PatternString();
	else
		return CPattern::szDefaultUserCommand;
}

//-------------------------------------------------------------------------------------
BOOL CPort::StartJob(DWORD nJobId, LPWSTR szJobTitle, LPWSTR szPrinterName)
{
	UNREFERENCED_PARAMETER(szJobTitle);

	_ASSERTE(m_pPattern != NULL);

	if (!m_pPattern)
		return FALSE;

	m_nJobId = nJobId;

	m_pPattern->Reset();

	//retrieve job info
	DWORD cbNeeded;

	CPrinterHandle printer(szPrinterName);

	if (!printer.Handle())
	{
		g_pLog->Critical(this, L"CPort::StartJob: OpenPrinterW failed (%i)", GetLastError());
		return FALSE;
	}

	//JOB_INFO_2
	GetJobW(printer, nJobId, 2, NULL, 0, &cbNeeded);

	if (!m_pJobInfo2 || m_cbJobInfo2 < cbNeeded)
	{
		if (m_pJobInfo2)
			delete[] m_pJobInfo2;

		m_cbJobInfo2 = cbNeeded;
		m_pJobInfo2 = reinterpret_cast<JOB_INFO_2W*>(new BYTE[cbNeeded]);
	}

	if (!GetJobW(printer, nJobId, 2, reinterpret_cast<LPBYTE>(m_pJobInfo2), m_cbJobInfo2, &cbNeeded))
	{
		g_pLog->Critical(this, L"CPort::StartJob: GetJobW failed (%i)", GetLastError());
		return FALSE;
	}

	//determine if a job was submitted locally by comparing local netbios name
	//with that stored into m_pJobInfo
	WCHAR szComputerName[256];
	DWORD nSize = LENGTHOF(szComputerName);

	GetComputerNameW(szComputerName, &nSize);

	m_bJobIsLocal = (
		m_pJobInfo2 &&
		(
			//the machine names are exactly the same...
			_wcsicmp(szComputerName, m_pJobInfo2->pMachineName) == 0 ||
			(
				//...or we have two extra \\ in the machine name provided by the spooler
				*m_pJobInfo2->pMachineName &&
				m_pJobInfo2->pMachineName[0] == L'\\' &&
				m_pJobInfo2->pMachineName[1] == L'\\' &&
				_wcsicmp(szComputerName, m_pJobInfo2->pMachineName + 2) == 0
			)
		)
	);

	//copy printer name locally
	size_t len = wcslen(szPrinterName) + 1;

	if (!m_szPrinterName || m_cchPrinterName < len)
	{
		if (m_szPrinterName)
			delete[] m_szPrinterName;

		m_cchPrinterName = static_cast<DWORD>(len);
		m_szPrinterName = new WCHAR[len];
	}

	wcscpy_s(m_szPrinterName, m_cchPrinterName, szPrinterName);

	//event to signal work to be done
	if (!m_hWorkEvt)
		if ((m_hWorkEvt = CreateEventW(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			g_pLog->Critical(this, L"CPort::StartJob: CreateEventW failed (%i)", GetLastError());
			return FALSE;
		}

	//event to signal work has been done
	if (!m_hDoneEvt)
		if ((m_hDoneEvt = CreateEventW(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			g_pLog->Critical(this, L"CPort::StartJob: CreateEventW failed (%i)", GetLastError());
			return FALSE;
		}

	//the writing thread - since we can't create an "overlapped pipe"
	//we use threads to mimick overlapped I/O, to avoid "waiting forever"
	//on a write to a broken pipe
	if (!m_hWriteThread)
	{
		DWORD dwId = 0;
		m_threadData.pPort = this;
		if ((m_hWriteThread = CreateThread(NULL, 0, WriteThreadProc, (LPVOID)&m_threadData, 0, &dwId)) == NULL)
			return FALSE;
		g_pLog->Debug(L"Worker thread started (id: 0x%0.8X)", dwId);
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------
DWORD CPort::CreateOutputFile()
{
	_ASSERTE(m_pPattern != NULL);

	if (!m_pPattern)
		return ERROR_CAN_NOT_COMPLETE;

	/*start composing the output filename*/
	wcscpy_s(m_szFileName, LENGTHOF(m_szFileName), m_szOutputPath);

	/*append a backslash*/
	size_t pos = wcslen(m_szFileName);
	if (pos == 0 || m_szFileName[pos - 1] != L'\\')
	{
		wcscat_s(m_szFileName, LENGTHOF(m_szFileName), L"\\");
		pos++;
	}

	/*the search algorithm uses search strings from "search fields", if any*/
	WCHAR szSearchPath[MAX_PATH + 1];
	wcscpy_s(szSearchPath, LENGTHOF(szSearchPath), m_szFileName);

	if (m_hToken)
	{
		if (!ImpersonateLoggedOnUser(m_hToken))
		{
			DWORD dwErr = GetLastError();
			g_pLog->Critical(L"CPort::CreateOutputFile: ImpersonateLoggedOnUser failed (%i)", dwErr);
			return dwErr;
		}
	}

	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwCreationDisposition;

	if (m_bOverwrite)
		dwCreationDisposition = CREATE_ALWAYS; // request that a new file be created
	else
		dwCreationDisposition = CREATE_NEW; // request that we're also the creators of the file

	/*start finding a file name*/
	do
	{
		m_szFileName[pos] = L'\0';
		szSearchPath[pos] = L'\0';

		/*get current value from pattern*/
		LPWSTR szFileName = m_pPattern->Value();
		LPWSTR szSearchName = m_pPattern->SearchValue();

		/*append it to output file name*/
		wcscat_s(m_szFileName, LENGTHOF(m_szFileName), szFileName);
		wcscat_s(szSearchPath, LENGTHOF(szSearchPath), szSearchName);

		/*check if parent directory exists*/
		GetFileParent(m_szFileName, m_szParent, LENGTHOF(m_szParent));

		if ((dwRet = RecursiveCreateFolder(m_szParent)) != ERROR_SUCCESS)
		{
			g_pLog->Critical(this, L"CPort::CreateOutputFile: can't create output directory (%i)", dwRet);
			dwRet = ERROR_DIRECTORY;
			goto cleanup;
		}

		//is this file name usable?
		//2009-08-04 we use search strings
//		if (!m_bOverwrite && FileExists(m_szFileName))
		/* moment A */
		if (!m_bOverwrite && FilePatternExists(szSearchPath))
			continue;

		//ok we got a valid filename, create it
		if (m_bPipeData)
		{
			if (!m_pUserCommand || !*m_pUserCommand->PatternString())
			{
				g_pLog->Critical(this, L"CPort::CreateOutputFile: empty user command, can't continue");
				dwRet = ERROR_CAN_NOT_COMPLETE;
				goto cleanup;
			}

			HANDLE hStdinR, hStdoutW, hStdoutR;
			SECURITY_ATTRIBUTES saAttr = { 0 };

			saAttr.nLength = sizeof(saAttr);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;

			//we have an external program to send data to
			//2009-06-12 batch files are executed through cmd.exe
			//with /C switch. cmd.exe won't start if we do not supply
			//a stdout handle
			if (!CreatePipe(&hStdinR, &m_hFile, &saAttr, 0) ||
				!CreatePipe(&hStdoutR, &hStdoutW, &saAttr, 0) ||
				!SetHandleInformation(m_hFile, HANDLE_FLAG_INHERIT, 0) ||
				!SetHandleInformation(hStdoutR, HANDLE_FLAG_INHERIT, 0))
			{
				g_pLog->Critical(this,
					L"CPort::CreateOutputFile: can't create pipes (%i)", GetLastError());
				dwRet = ERROR_FILE_INVALID;
				goto cleanup;
			}

			STARTUPINFOW si = { 0 };
			si.cb = sizeof(si);
			si.hStdInput = hStdinR;
			si.hStdOutput = hStdoutW;
			si.hStdError = hStdoutW;
			if (m_bHideProcess)
			{
				si.wShowWindow = SW_HIDE;
			}
			else
			{
				si.wShowWindow = SW_SHOW;
				si.lpDesktop = _wcsdup(L"winsta0\\default");
			}
			si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

			//create child process - give up in case of failure since we need to write to process
			BOOL bRes;
			if (m_hToken)
				bRes = CreateProcessAsUserW(m_hToken, NULL, m_pUserCommand->Value(), NULL, NULL,
					TRUE, 0, NULL, (*m_szExecPath) ? m_szExecPath : NULL, &si, &m_procInfo);
			else
				bRes = CreateProcessW(NULL, m_pUserCommand->Value(), NULL, NULL,
					TRUE, 0, NULL, (*m_szExecPath) ? m_szExecPath : NULL, &si, &m_procInfo);

			DWORD dwErr = GetLastError();

			if (si.lpDesktop)
				free(si.lpDesktop);

			//close stdout and stdin pipe after child process has inherited them
			CloseHandle(hStdoutW);
			CloseHandle(hStdinR);

			if (!bRes)
			{
				g_pLog->Critical(this, L"CPort::CreateOutputFile: CreateProcessW failed (%i)", dwErr);
				g_pLog->Info(L" User command = %s", m_pUserCommand->Value());
				g_pLog->Info(L" Execute from = %s", m_szExecPath);

				WCHAR szBuf[128];
				DWORD dwCb = LENGTHOF(szBuf);
				GetUserNameW(szBuf, &dwCb);
				g_pLog->Info(L" Running as   = %s", szBuf);

				CloseHandle(hStdoutR);

				dwRet = ERROR_CAN_NOT_COMPLETE;
				goto cleanup;
			}

			m_bPipeActive = TRUE;

			//start reading thread - the thread will read and discard anything that comes from
			//the external program, and finally close handle to our end of stdout
			HANDLE hReadThread = NULL;
			DWORD dwId = 0;
			if ((hReadThread = CreateThread(NULL, 0, ReadThreadProc, static_cast<LPVOID>(hStdoutR), 0, &dwId)) == NULL)
			{
				g_pLog->Critical(this, L"CPort::CreateOutputFile: CreateThread failed (%i)", GetLastError());
				dwRet = ERROR_CAN_NOT_COMPLETE;
				goto cleanup;
			}

			//we immediately close handle to the thread and leave it
			//around making its job until it's done
			CloseHandle(hReadThread);

			goto cleanup;
		}
		else
		{
			//output on a regular file
			/* moment B */
			m_hFile = CreateFileW(m_szFileName, GENERIC_WRITE, 0,
				NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

			if (m_hFile == INVALID_HANDLE_VALUE)
			{
				//did somebody already create the file between moment A and moment B?
				if (!m_bOverwrite && GetLastError() == ERROR_FILE_EXISTS)
					continue;

				g_pLog->Critical(this, L"CPort::CreateOutputFile: CreateFileW failed (%i)", GetLastError());
				dwRet = ERROR_FILE_INVALID;
			}

			goto cleanup;
		}
	} while (m_pPattern->NextValue()); //loop until there are no more combinations for pattern

	g_pLog->Critical(this, L"CPort::CreateOutputFile: can't get a valid filename");

	dwRet = ERROR_FILE_EXISTS;

cleanup:
	if (m_hToken)
		RevertToSelf();

	return dwRet;
}

//-------------------------------------------------------------------------------------
BOOL CPort::WriteToFile(LPCVOID lpBuffer, DWORD cbBuffer, LPDWORD pcbWritten)
{
	//if we're writing to a pipe, make sure proces is alive
	if (m_bPipeActive)
	{
		DWORD dwCode = 0;

		if (!GetExitCodeProcess(m_procInfo.hProcess, &dwCode) ||
			dwCode != STILL_ACTIVE)
		{
			m_bPipeActive = FALSE;
			SetLastError(ERROR_CAN_NOT_COMPLETE);
			return FALSE;
		}
	}

	//pass buffer to the write thread
	m_threadData.lpBuffer = lpBuffer;
	m_threadData.cbBuffer = cbBuffer;
	m_threadData.pcbWritten = pcbWritten;

	//wake up thread
	SetEvent(m_hWorkEvt);

	for (;;)
	{
		switch (WaitForSingleObject(m_hDoneEvt, 10000))
		{
		case WAIT_OBJECT_0:
			return TRUE;
			break;
		case WAIT_TIMEOUT:
			if (!m_bJobIsLocal || MessageBoxW(GetDesktopWindow(), szMsgUserCommandLocksSpooler, szAppTitle, MB_YESNO) == IDNO)
			{
				TerminateThread(m_hWriteThread, 1);
				CloseHandle(m_hWriteThread);
				m_hWriteThread = NULL;
				return FALSE;
			}
			break;
		default:
			return FALSE;
		}
	}
}

//-------------------------------------------------------------------------------------
DWORD WINAPI CPort::WriteThreadProc(LPVOID lpParam)
{
	LPTHREADDATA pData = static_cast<LPTHREADDATA>(lpParam);

	_ASSERTE(pData != NULL);
	_ASSERTE(pData->pPort != NULL);

	for (;;)
	{
		//wait signal from main thread
		if (WaitForSingleObject(pData->pPort->m_hWorkEvt, INFINITE) == WAIT_OBJECT_0)
		{
			EnterCriticalSection(&pData->csBuffer);

			if (pData->lpBuffer == NULL)
			{
				LeaveCriticalSection(&pData->csBuffer);
				SetEvent(pData->pPort->m_hDoneEvt);
				return 0;
			}

			pData->bStatus = WriteFile(pData->pPort->m_hFile, pData->lpBuffer, pData->cbBuffer,
				pData->pcbWritten, NULL);

			LeaveCriticalSection(&pData->csBuffer);
		}

		//signal we're done
		SetEvent(pData->pPort->m_hDoneEvt);
	}
}

//-------------------------------------------------------------------------------------
DWORD WINAPI CPort::ReadThreadProc(LPVOID lpParam)
{
	HANDLE hStdoutR = static_cast<HANDLE>(lpParam);

	_ASSERTE(hStdoutR != NULL);

	char buf[512] = { 0 };
	DWORD dwRead;

	//we read and simply trash anything that comes from the user command
	for (;;)
	{
		BOOL bRes;
		bRes = ReadFile(hStdoutR, buf, sizeof(buf), &dwRead, NULL);
		if (!bRes || dwRead == 0)
			break;
	}

	//done, close handle to our end of the pipe
	CloseHandle(hStdoutR);

	return 0;
}

//-------------------------------------------------------------------------------------
BOOL CPort::EndJob()
{
	_ASSERTE(m_pPattern != NULL);

	if (!m_pPattern)
		return FALSE;

	//done with the file, close it and flush buffers
	FlushFileBuffers(m_hFile);
	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_bPipeActive = FALSE;

	//tell the spooler we are done with the job
	CPrinterHandle printer(m_szPrinterName);

	if (printer.Handle())
		SetJobW(printer, JobId(), 0, NULL, JOB_CONTROL_DELETE);

	//start user command
	if (!m_bPipeData && m_pUserCommand && *m_pUserCommand->PatternString())
	{
		STARTUPINFOW si = { 0 };

		ZeroMemory(&m_procInfo, sizeof(m_procInfo));

		si.cb = sizeof(si);

		//we're not going to give up in case of failure
		if (m_hToken)
			CreateProcessAsUserW(m_hToken, NULL, m_pUserCommand->Value(), NULL, NULL,
				FALSE, 0, NULL, (*m_szExecPath) ? m_szExecPath : NULL, &si, &m_procInfo);
		else
			CreateProcessW(NULL, m_pUserCommand->Value(), NULL, NULL,
				FALSE, 0, NULL, (*m_szExecPath) ? m_szExecPath : NULL, &si, &m_procInfo);
	}

	//maybe wait and close handles to child process
	if (m_procInfo.hProcess)
	{
		if (m_bWaitTermination)
		{
			BOOL bDone = FALSE;

			while (!bDone)
			{
				switch (WaitForSingleObject(m_procInfo.hProcess, m_dwWaitTimeout ? m_dwWaitTimeout * 1000 : INFINITE))
				{
				case WAIT_OBJECT_0:
					bDone = TRUE;
					break;
				case WAIT_TIMEOUT:
					if (!m_bJobIsLocal || MessageBoxW(GetDesktopWindow(), szMsgUserCommandLocksSpooler, szAppTitle, MB_YESNO) == IDNO)
						bDone = TRUE;
					break;
				}
			}
		}
		CloseHandle(m_procInfo.hProcess);
		CloseHandle(m_procInfo.hThread);
	}

	*m_szFileName = L'\0';

	return TRUE;
}

//-------------------------------------------------------------------------------------
void CPort::SetConfig(LPPORTCONFIG pConfig)
{
	g_pLog->SetLogLevel(pConfig->nLogLevel);
	
	if (m_hToken)
	{
		CloseHandle(m_hToken);
		m_hToken = NULL;
	}
	
	Initialize(pConfig);
	m_bLogonInvalidated = TRUE;

	if (Logon() != ERROR_SUCCESS)
	{
		g_pLog->Error(this, L"CPort::SetConfig: can't logon user");
	}
}

//-------------------------------------------------------------------------------------
LPCWSTR CPort::ComputerName() const
{
	if (m_pJobInfo2)
	{
		//strip backslashes off
		LPWSTR pBuf = m_pJobInfo2->pMachineName;

		while (*pBuf == L'\\')
			pBuf++;

		return pBuf;
	}
	else
		return L"";
}

//-------------------------------------------------------------------------------------
LPCWSTR CPort::Bin() const
{
	static WCHAR szBinName[16];

	if (!m_pJobInfo2 || !m_pJobInfo2->pDevMode || (m_pJobInfo2->pDevMode->dmFields & DM_DEFAULTSOURCE) == 0)
		return L"";

	switch (m_pJobInfo2->pDevMode->dmDefaultSource)
	{
	case DMBIN_AUTO:
		return L"AUTO";
	case DMBIN_CASSETTE:
		return L"CASSETTE";
	case DMBIN_ENVELOPE:
		return L"ENVELOPE";
	case DMBIN_ENVMANUAL:
		return L"ENVMANUAL";
	//case DMBIN_FIRST:
	//	return L"FIRST";
	case DMBIN_FORMSOURCE:
		return L"FORMSOURCE";
	case DMBIN_LARGECAPACITY:
		return L"LARGECAPACITY";
	case DMBIN_LARGEFMT:
		return L"LARGEFMT";
	//case DMBIN_LAST:
	//	return L"LAST";
	case DMBIN_LOWER:
		return L"LOWER";
	case DMBIN_MANUAL:
		return L"MANUAL";
	case DMBIN_MIDDLE:
		return L"MIDDLE";
	//case DMBIN_ONLYONE:
	//	return L"ONLYONE";
	case DMBIN_TRACTOR:
		return L"TRACTOR";
	case DMBIN_SMALLFMT:
		return L"SMALLFMT";
	case DMBIN_UPPER:
		return L"UPPER";
	default:
		if (m_pJobInfo2->pDevMode->dmDefaultSource >= DMBIN_USER)
		{
			swprintf_s(szBinName, LENGTHOF(szBinName), L"USER%hi", m_pJobInfo2->pDevMode->dmDefaultSource);
		}
		else
		{
			swprintf_s(szBinName, LENGTHOF(szBinName), L"%hi", m_pJobInfo2->pDevMode->dmDefaultSource);
		}
		return szBinName;
	}
}

//-------------------------------------------------------------------------------------
DWORD CPort::Logon()
{
	if (!m_bLogonInvalidated)
		return ERROR_SUCCESS;

	if (m_hToken)
	{
		CloseHandle(m_hToken);
		m_hToken = NULL;
	}

	if (!*m_szUser)
		return ERROR_SUCCESS;

	BOOL bUNC = wcschr(m_szUser, L'@') != NULL;

	if (!bUNC && !*m_szDomain)
	{
		g_pLog->Error(this, L"CPort::Logon: empty domain");
		return ERROR_BAD_ARGUMENTS;
	}

	if (!GetPrimaryToken(m_szUser, bUNC ? NULL : m_szDomain, m_szPassword, &m_hToken, &m_bRestrictedToken))
	{
		DWORD dwErr = GetLastError();
		g_pLog->Error(this, L"CPort::Logon: GetPrimaryToken failed - user = \"%s\", domain = \"%s\" (%i)",
			m_szUser,
			bUNC ? m_szDomain : L"<empty>",
			dwErr);
		return dwErr;
	}

	m_bLogonInvalidated = FALSE;

	return ERROR_SUCCESS;
}

//-------------------------------------------------------------------------------------
DWORD CPort::RecursiveCreateFolder(LPCWSTR szPath)
{
	WCHAR szPathBuf[MAX_PATH + 1];
	WCHAR szParent[MAX_PATH + 1];
	LPCWSTR pPath = szPath;
	size_t len;

	/*strip off leading backslashes*/
	len = wcslen(szPath);
	if (len > 0 && ISSLASH(szPath[len - 1]))
	{
		/*make a copy of szPath only if needed*/
		wcscpy_s(szPathBuf, LENGTHOF(szPathBuf), szPath);
		pPath = szPathBuf;
		while (len > 0 && ISSLASH(szPathBuf[len - 1]))
		{
			szPathBuf[len - 1] = L'\0';
			len--;
		}
	}
	/*only drive letter left or the directory already exists*/
	if (len < 3 || DirectoryExists(pPath))
		return ERROR_SUCCESS;
	else
	{
		GetFileParent(pPath, szParent, LENGTHOF(szParent));
		if (wcscmp(pPath, szParent) == 0)
			return ERROR_SUCCESS;
		/*our parent must exist before we can get created*/
		DWORD dwRet = RecursiveCreateFolder(szParent);
		if (dwRet != ERROR_SUCCESS)
			return dwRet;
		if (!CreateDirectoryW(pPath, NULL))
			return GetLastError();
		return ERROR_SUCCESS;
	}
}

//-------------------------------------------------------------------------------------
DWORD CPort::CreateOutputPath()
{
	if (m_hToken)
	{
		if (!ImpersonateLoggedOnUser(m_hToken))
		{
			DWORD dwErr = GetLastError();
			g_pLog->Critical(L"CPort::CreateOutputPath: ImpersonateLoggedOnUser failed (%i)", dwErr);
			return dwErr;
		}
	}

	DWORD dwRet = RecursiveCreateFolder(m_szOutputPath);

	if (m_hToken)
		RevertToSelf();

	return dwRet;
}
