/*
http://code.google.com/p/wxxkey/
Copyright (C) 2012  Just Fancy (weitianleung@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//========================================================================================================
#include "common.h"

#include <Tlhelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "Version.lib")
//========================================================================================================
extern WarVer g_warVer ;
//========================================================================================================
static UINT_PTR CALLBACK OFNHookProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL	GetDllVersion(LPCTSTR lpFile, PDWORD pdwMajorVersion, PDWORD pdwMinorVersion, PDWORD pdwBuildNumber);
static BOOL ParseVersionString(LPTSTR lpVersion, PDWORD pdwMajorVersion, PDWORD pdwMinorVersion, PDWORD pdwBuildNumber);

static BOOL GetWar3PathByReg(LPTSTR lpBuffer, int nMax);
static BOOL GetWar3PathByMem(LPTSTR lpBuffer, int nMax);
//========================================================================================================
void SetFont(HWND hWnd, LPCTSTR lpFace, int size)
{
	HFONT hFont = CreateFont(
		size, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH, lpFace
		);

	if (hFont)
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
}
//========================================================================================================
void ShowPopupMenu(HWND hWnd, UINT uID)
{
	HMENU hMenu = 0;
	HMENU hMenuTP = 0;
	POINT pt;

	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(uID));
	hMenuTP = GetSubMenu(hMenu, 0);

	if (!hMenu)	return ;

	GetCursorPos(&pt);
	TrackPopupMenuEx(hMenuTP, TPM_BOTTOMALIGN, pt.x, pt.y, hWnd, 0);

	DestroyMenu(hMenu);
}
//========================================================================================================
static BOOL GetWar3PathByReg(LPTSTR lpBuffer, int nMax)
{
	HKEY hKey;
	LSTATUS ret;
	DWORD cbData = nMax;

	if (lpBuffer == NULL || nMax <= 0)
		return FALSE;

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Blizzard Entertainment\\Warcraft III"),
		0, KEY_READ, &hKey);

	if (ERROR_SUCCESS != ret)
		return FALSE;

	ret = RegQueryValueEx(hKey, _T("GamePath"), NULL, NULL, (LPBYTE)lpBuffer, &cbData);
	RegCloseKey(hKey);

	if (ERROR_SUCCESS != ret)
		return FALSE;

	return cbData > 0;
}
//========================================================================================================
static BOOL GetWar3PathByMem(LPTSTR lpBuffer, int nMax)
{
	DWORD dwPID;
//	HANDLE hProcess;
//	TCHAR szPath[MAX_PATH] = { 0 };

	HANDLE hSnapshot;
	MODULEENTRY32 me32 = {0};

	if (NULL == lpBuffer || nMax < 0)
		return FALSE;

	dwPID = GetWar3PID();
	if (0 == dwPID)
		return FALSE;

/*	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);

	if (NULL == hProcess)
		return FALSE;
*/
	// 这函数得OS 高于 Vista 囧
	// QueryFullProcessImageName(hProcess, 0, lpBuffer, (PDWORD)&nMax);

//	CloseHandle(hProcess);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		return FALSE;

	me32.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hSnapshot, &me32))
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}

	CloseHandle(hSnapshot);

	_tcscpy_s(lpBuffer, nMax, me32.szExePath);

	return TRUE;
}
//========================================================================================================
BOOL GetWar3Path(LPTSTR lpBuffer, int nMax)
{
	if (!GetWar3PathByMem(lpBuffer, nMax))
		return GetWar3PathByReg(lpBuffer, nMax);

	return TRUE;
}
//========================================================================================================
HANDLE RunApp(LPTSTR lpExeFile)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	GetStartupInfo(&si);

	if (!CreateProcess(NULL, lpExeFile, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		return INVALID_HANDLE_VALUE;

	return pi.hProcess;
}
//========================================================================================================
DWORD GetWar3PID()
{
	HWND hWnd = FindWindow(_T("Warcraft III"), _T("Warcraft III"));

	if (NULL != hWnd)
	{
		DWORD dwPID = 0;

		GetWindowThreadProcessId(hWnd, &dwPID);

		return dwPID;
	}

	return 0;
}
//========================================================================================================
BOOL SelectOneFile(HWND hWnd, LPTSTR lpBuffer, int nMax)
{
	OPENFILENAME ofn = {0};

	if (NULL == lpBuffer || nMax <= 0)
		return FALSE;

	RtlSecureZeroMemory(lpBuffer, nMax);

	lstrcpy(lpBuffer, _T("War3.exe"));

	ofn.hInstance 	 = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	ofn.hwndOwner 	 = hWnd;
	ofn.Flags 		 = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLEHOOK;
	ofn.lpstrFile	 = lpBuffer;
	ofn.nMaxFile 	 = nMax;
	ofn.lStructSize  = sizeof(OPENFILENAME);
	ofn.lpstrFilter  = TEXT("War3.exe\0War3.exe\0\0");
	ofn.lpstrTitle   = TEXT("请选择War3.exe");
	ofn.lpfnHook	 = OFNHookProc;

	if (GetOpenFileName(&ofn))
		return TRUE;

	return FALSE;
}
//========================================================================================================
static UINT_PTR CALLBACK OFNHookProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		EnableWindow( GetDlgItem( GetParent( hDlg ), cmb13 ), FALSE );
		return 0;
	}

	return 0;
}
//========================================================================================================
BOOL IsGameStated()
{
	return TRUE;
}
//========================================================================================================
BOOL IsChatting()
{
	HANDLE hProcess;
	BOOL fChatting = FALSE;
	DWORD base = 0;

	hProcess = OpenProcess(PROCESS_VM_READ, FALSE, GetWar3PID());

	if (NULL == hProcess)
		return fChatting;

	if (g_warVer == vUnset)
		g_warVer = GetWar3Version();

	switch(g_warVer)
	{
	case v120e:
		base = 0x45CB8C;
		break;

	case v124a:
	case v124b:
	case v124c:
	case v124d:
	case v124e:
		base = 0x6FAE8450;
		break;

	case v125b:
	case v126a:
		base = 0x6FAD15F0;
		break;
	}

	if (g_warVer != vUnknown)
		ReadProcessMemory(hProcess, (LPCVOID)base, (LPVOID)&fChatting, sizeof(fChatting), NULL);

	CloseHandle(hProcess);

	return fChatting;
}
//========================================================================================================
BOOL EnableDebugPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp = {0};

	HANDLE hProcess = GetCurrentProcess();

	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
		return FALSE;


	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
		return FALSE;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, 0))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return FALSE;

	return TRUE;
}
//========================================================================================================
WarVer GetWar3Version()
{
	TCHAR szDllPath[MAX_PATH] = {0};
	DWORD dwMajor, dwMinor, dwBuild;

	if (!GetWar3Path(szDllPath, MAX_PATH))
		return vUnknown;

	PathRemoveFileSpec(szDllPath);

	_tcscat_s(szDllPath, MAX_PATH, _T("\\Game.dll"));

	if (!GetDllVersion(szDllPath, &dwMajor, &dwMinor, &dwBuild))
		return vUnknown;

	if (dwMajor == 1)
	{
		if (dwMinor == 20)
		{
			switch(dwBuild)
			{
			case 4:	return v120e;
			}
		}
		else if(dwMinor == 24)
		{
			switch(dwBuild)
			{
			case 0:	return v124a;
			case 1:	return v124b;
			case 2:	return v124c;
			case 3:	return v124d;
			case 4:	return v124e;
			}
		}
		else if(dwMinor == 25)
		{
			switch(dwBuild)
			{
			case 1:	return v125b;
			}
		}
		else if(dwMinor == 26)
		{
			switch(dwBuild)
			{
			case 0:	return v126a;
			}
		}
	}

	return vUnknown;
}

//========================================================================================================
DWORD GetGameDLLBase(DWORD dwWar3PID)
{
	HANDLE hSnapshot;
	MODULEENTRY32 me32 = {0};

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwWar3PID);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		return 0;

	me32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &me32))
	{
		do
		{
			if (lstrcmpi(_T("Game.dll"), me32.szModule) == 0)
			{
				CloseHandle(hSnapshot);

				return (DWORD) me32.modBaseAddr;
			}

		}while(Module32Next(hSnapshot, &me32));
	}

	CloseHandle(hSnapshot);

	return 0;
}
//========================================================================================================
static BOOL	GetDllVersion(LPCTSTR lpFile, PDWORD pdwMajorVersion, PDWORD pdwMinorVersion, PDWORD pdwBuildNumber)
{
	DWORD dwSize;
	PVOID buffer = NULL;
	TCHAR szVer[50] = { 0 };

	LANGANDCODEPAGE *lpTranslate = NULL;
	UINT cbSize = sizeof(LANGANDCODEPAGE);

	BOOL bStatus = FALSE;

	dwSize = GetFileVersionInfoSize(lpFile, NULL);
	if (dwSize <= 0)
		return FALSE;

	buffer = malloc(dwSize);
	if (NULL == buffer)
		return FALSE;

	do 
	{
		int i;
		int count;

		if (!GetFileVersionInfo(lpFile, 0, dwSize, buffer))
			break;

		if (!VerQueryValue(buffer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbSize))
			break;

		count = cbSize / sizeof(LANGANDCODEPAGE);

		for( i=0; i < count; i++ )
		{
			TCHAR szSubBlock[50];
			LPVOID version = NULL;

			wsprintf(szSubBlock, _T("\\StringFileInfo\\%04x%04x\\FileVersion"),
				lpTranslate[i].wLanguage,
				lpTranslate[i].wCodePage);

			cbSize = sizeof(LANGANDCODEPAGE);

			if (VerQueryValue(buffer, szSubBlock, &version, &cbSize))
			{
				lstrcpy(szVer, (TCHAR*)version);

				bStatus = TRUE;
				break;
			}
		}

	} while (0);

	if (bStatus)
	{
		bStatus = ParseVersionString(szVer, pdwMajorVersion, pdwMinorVersion, pdwBuildNumber);
	}

	free(buffer);

	return bStatus;
}
//========================================================================================================
static BOOL ParseVersionString(LPTSTR lpVersion, PDWORD pdwMajorVersion, PDWORD pdwMinorVersion, PDWORD pdwBuildNumber)
{
	TCHAR *token;
	TCHAR *next_token = NULL;

	TCHAR szDelimit[2] = { _T(",") };

	if (NULL == lpVersion)
		return FALSE;

	token = _tcstok_s(lpVersion, szDelimit, &next_token);
	if (NULL == token)
	{
		token = _tcstok_s(lpVersion, _T("."), &next_token);
		if (NULL == token)
			return FALSE;

		szDelimit[0] = _T('.');
	}

 	if (pdwMajorVersion)
		*pdwMajorVersion = _tstoi(token);

	token = _tcstok_s(NULL, szDelimit, &next_token);
	if (token == NULL)
		return FALSE;

	if (pdwMinorVersion)
		*pdwMinorVersion = _tstoi(token);

	token = _tcstok_s(NULL, szDelimit, &next_token);
	if (token == NULL)
		return FALSE;

	if (pdwBuildNumber)
		*pdwBuildNumber = _tstoi(token);

	return TRUE;
}
//========================================================================================================
