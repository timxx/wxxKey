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
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
//========================================================================================================
#define HANDLE_MM_TRAY(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (wParam), (lParam)), 0L)

#define HANDLE_MM_WAITAPP(hwnd, wParam, lParam, fn)	\
	((fn)((hwnd)), 0L)
//========================================================================================================
#include "wxxKey.h"
#include "tabctl.h"
#include "dlgproc.h"
#include "wkey.h"
#include "mm.h"

#include <commctrl.h>
#include <shlwapi.h>

//========================================================================================================

//========================================================================================================
static DWORD WINAPI WaitForApp(LPVOID lParam);
//========================================================================================================
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };

	// 仅允许运行一个实例
	CreateMutex(NULL, FALSE, _T("wxxKeyMutex"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(szClsName, NULL);

		if (NULL != hWnd)
		{
			SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_RESTORE, 0), 0);
		}

		return 0;
	}

	if (!Register(hInstance) || !InitWindow(hInstance))
		return 0;

	if (!EnableDebugPrivilege())
	{
// 		MessageBox(NULL, _T("EnableDebugPrivilege failed!"), _T("Error"), MB_ICONERROR);
	}

	g_hhook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(g_hDlgWarkey, &msg) &&
			!IsDialogMessage(g_hDlgSett, &msg) &&
			!IsDialogMessage(g_hDlgAbout, &msg)
			)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnhookWindowsHookEx(g_hhook);

	return (int)msg.wParam;
}
//========================================================================================================
ATOM Register(HINSTANCE hInst)
{
	WNDCLASSEX wcex = {0};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = GetStockBrush(WHITE_BRUSH);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hInstance = hInst;
	wcex.lpszClassName = szClsName;
	wcex.lpszMenuName = NULL;
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;

	return RegisterClassEx(&wcex);
}
//========================================================================================================
BOOL InitWindow(HINSTANCE hInst)
{
	HWND hWnd = CreateWindowEx(0, szClsName, szWndTitle,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, 
		0, 0, 390, 430,
		NULL, NULL, hInst, NULL);

	if (!hWnd)
		return FALSE;

	g_hwndMain = hWnd;

	CenterWnd(hWnd, GetDesktopWindow());

	if (g_fTrayOnly)
	{
		ShowTray(TRUE);
		Shell_NotifyIcon(NIM_SETFOCUS, &g_nid);
	}
	else
	{
		ShowWindow(hWnd, SW_SHOW);
	}

	UpdateWindow(hWnd);

	return TRUE;
}
//========================================================================================================
void CenterWnd(HWND hwndTo, HWND hwndParent)
{
	RECT rcParent;
	RECT rcTo;
	int x, y;
	int nWidth, nHeight;

	GetWindowRect(hwndParent, &rcParent);
	GetWindowRect(hwndTo, &rcTo);

	nWidth = rcTo.right - rcTo.left;
	nHeight = rcTo.bottom - rcTo.top;

	x = rcParent.left + (rcParent.right - rcParent.left - nWidth)/2;
	y = rcParent.top + (rcParent.bottom - rcParent.top - nHeight)/2;

	MoveWindow(hwndTo, x, y, nWidth, nHeight, TRUE);
}
//========================================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_CLOSE,		OnClose);
		HANDLE_MSG(hWnd, WM_CREATE,		OnCreate);
		HANDLE_MSG(hWnd, WM_NOTIFY,		OnNotify);
		HANDLE_MSG(hWnd, WM_COMMAND,	OnCommand);
		HANDLE_MSG(hWnd, WM_SYSCOMMAND,	OnSysCommand);
		HANDLE_MSG(hWnd, MM_TRAY,		OnTray);
		HANDLE_MSG(hWnd, MM_WAITAPP,	OnWaitApp);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
//========================================================================================================
void OnClose(HWND hWnd)
{
	TCHAR szTemp[MAX_PATH] = {0};

	GetDlgItemText(g_hDlgSett, IDC_EDIT_PATH, szTemp, MAX_PATH);
	WritePrivateProfileString(_T("App"), _T("Path"), szTemp, g_szIniFile);

	SendMessage(g_hDlgWarkey, WM_CLOSE, 0, 0);
	SendMessage(g_hDlgSett, WM_CLOSE, 0, 0);
	SendMessage(g_hDlgAbout, WM_CLOSE, 0, 0);

	FreeWKey(g_listWarkey);

	ShowTray(FALSE);

	wsprintf(szTemp, _T("%d"), g_usingSLN);

	WritePrivateProfileString(_T("App"), _T("Using"), szTemp, g_szIniFile);

	if (g_hThread != INVALID_HANDLE_VALUE)
	{
		if (WaitForSingleObject(g_hThread, 100) == WAIT_TIMEOUT)
			TerminateThread(g_hThread, 0);

		CloseHandle(g_hThread);
	}

	DestroyWindow(hWnd);
	PostQuitMessage(0);
}
//========================================================================================================
BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	INITCOMMONCONTROLSEX icce = { 0 };
	RECT rc;

	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;

	InitCommonControlsEx(&icce);

	GetClientRect(hWnd, &rc);

	g_hwndTab = TabCtl_Create(hWnd, 0, 0, rc.right - rc.left, 20);

	SetFont(g_hwndTab, _T("宋体"), 12);

	TabCtl_Append(g_hwndTab, _T("魔键方案"));
	TabCtl_Append(g_hwndTab, _T("程序设置"));
	TabCtl_Append(g_hwndTab, _T("帮助说明"));

	g_hDlgWarkey = CreateDialog(lpCreateStruct->hInstance, MAKEINTRESOURCE(IDD_WAR_KEY), hWnd, DlgWarkeyProc);
	g_hDlgSett	 = CreateDialog(lpCreateStruct->hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, DlgSettingsProc);
	g_hDlgAbout	 = CreateDialog(lpCreateStruct->hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, DlgAboutProc);

	SetWindowPos(g_hDlgWarkey, NULL, rc.left, rc.top + 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(g_hDlgSett, NULL, rc.left, rc.top + 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(g_hDlgAbout, NULL, rc.left, rc.top + 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	GetModuleFileName(NULL, g_szIniFile, MAX_PATH);
	PathRemoveFileSpec(g_szIniFile);
	lstrcat(g_szIniFile, _T("\\wxxKey.ini"));

	LoadSettings(g_szIniFile);

	if (LoadFromFile(g_szIniFile, &g_listWarkey))
	{
		int i;
		TCHAR szTemp[128] = {0};
		HWND hwndCb = NULL;

		int count = list_length(g_listWarkey);

		hwndCb = GetDlgItem(g_hDlgWarkey, IDC_COMBO_SLN);

		for(i=1; i<=count; i++)
		{
			wsprintf(szTemp, _T("方案%d"), i);

			SendMessage(hwndCb, CB_ADDSTRING, 0, (LPARAM)szTemp);
		}

		if (g_usingSLN >= (UINT)count)
			g_usingSLN = 0;

		SendMessage(hwndCb, CB_SETCURSEL, g_usingSLN, 0);

		PostMessage(g_hDlgWarkey, MM_SHOWSLN, 0, 0);
	}

	CheckDlgButton(g_hDlgSett, IDC_CHECK_TRAY, g_fTrayOnly);
	CheckDlgButton(g_hDlgSett, IDC_CHECK_HIDE, g_fHideWnd);

	ShowWindow(g_hDlgWarkey, SW_SHOW);

	InitTray(hWnd);

	return TRUE;
}
//========================================================================================================
LRESULT OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr)
{
	if(lpnmhdr->code == TCN_SELCHANGE)
	{
		switch(TabCtl_GetCurSel(g_hwndTab))
		{
		case 0:
			ShowWindow(g_hDlgWarkey, SW_SHOW);
			ShowWindow(g_hDlgSett, SW_HIDE);
			ShowWindow(g_hDlgAbout, SW_HIDE);
			break;

		case 1:
			ShowWindow(g_hDlgWarkey, SW_HIDE);
			ShowWindow(g_hDlgSett, SW_SHOW);
			ShowWindow(g_hDlgAbout, SW_HIDE);
			break;

		case 2:
			ShowWindow(g_hDlgWarkey, SW_HIDE);
			ShowWindow(g_hDlgSett, SW_HIDE);
			ShowWindow(g_hDlgAbout, SW_SHOW);
			break;
		}
	}

	return 0;
}
//========================================================================================================
void OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDM_EXIT:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case IDM_RESTORE:
		
		ShowWindow(hWnd, SW_SHOW);
		//SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetForegroundWindow(hWnd);

		ShowTray(FALSE);

		break;
	}
}
//========================================================================================================
void OnSysCommand(HWND hWnd, UINT cmd, int x, int y)
{
	if (cmd == SC_MINIMIZE && g_fHideWnd)
	{
		ShowTray(TRUE);
		ShowWindow(hWnd, SW_HIDE);

		Shell_NotifyIcon(NIM_SETFOCUS, &g_nid);
	}
	else
	{
		DefWindowProc(hWnd, WM_SYSCOMMAND, cmd, MAKELPARAM(x, y));
	}
}
//========================================================================================================
void OnTray(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDI_APP)
	{
		if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
		{
			ShowPopupMenu(hWnd, IDR_MENU_TRAY);
		}
		else if(lParam == WM_LBUTTONDBLCLK)
		{
			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);

			ShowTray(FALSE);
		}
	}
}
//========================================================================================================
void OnWaitApp(HWND hWnd)
{
	if (g_hThread == INVALID_HANDLE_VALUE)
	{
		DWORD dwTID;
		g_hThread = CreateThread(NULL, 0, WaitForApp, (LPVOID)INVALID_HANDLE_VALUE, 0, &dwTID);
	}
}
//========================================================================================================
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
		if (p->vkCode == VK_HOME)
		{
			g_fActived = !g_fActived;
		}
		else if(p->vkCode == VK_PRIOR)
		{
			INT_PTR nextSLN = g_usingSLN;

			if (nextSLN == 0)
			{
				nextSLN = list_length(g_listWarkey) - 1;
			}
			else
			{
				nextSLN--;
			}

			SendDlgItemMessage(g_hDlgWarkey, IDC_COMBO_SLN, CB_SETCURSEL, nextSLN, 0);
			PostMessage(g_hDlgWarkey, WM_COMMAND, MAKEWPARAM(IDC_COMBO_SLN, CBN_SELCHANGE), 0);
		}
		else if(p->vkCode == VK_NEXT)
		{
			INT_PTR nextSLN = g_usingSLN;

			if (nextSLN == list_length(g_listWarkey) - 1)
			{
				nextSLN = 0;
			}
			else
			{
				nextSLN++;
			}

			SendDlgItemMessage(g_hDlgWarkey, IDC_COMBO_SLN, CB_SETCURSEL, nextSLN, 0);
			PostMessage(g_hDlgWarkey, WM_COMMAND, MAKEWPARAM(IDC_COMBO_SLN, CBN_SELCHANGE), 0);
		}
		else if(g_fActived)
		{
			if (HookIt(p))
				return TRUE;
		}
	}

	return CallNextHookEx(g_hhook, nCode, wParam, lParam);
}
//========================================================================================================
BOOL HookIt(PKBDLLHOOKSTRUCT pkb)
{
	wKey *wkey;
	int i;
	List *p;

	static int numpad[6] =
	{
		VK_NUMPAD7, VK_NUMPAD8,
		VK_NUMPAD4, VK_NUMPAD5,
		VK_NUMPAD1, VK_NUMPAD2
	};

	HWND hWnd = FindWindow(_T("Warcraft III"), _T("Warcraft III"));

	if (GetForegroundWindow() != hWnd)
		return FALSE;

	if (!IsGameStated() || IsChatting())
		return FALSE;
	
	if (HIWORD(GetKeyState(VK_CONTROL)) || 
		HIWORD(GetKeyState(VK_SHIFT))	||
		HIWORD(GetKeyState(VK_MENU))
		)
		return FALSE;

	wkey = (wKey*)list_nth_data(g_listWarkey, g_usingSLN);
	if (NULL == wkey)
		return FALSE;

	for(i=0; i<6; i++)
	{
		if (wkey->Numpad[i] == pkb->vkCode)
		{
			SendKey(numpad[i]);
			return TRUE;
		}
	}

	/* *** */

	p = wkey->Combine;

	while (p)
	{
		cKey *ckey = (cKey *)p->data;
		if (ckey->key == pkb->vkCode)
		{
			List *pp = ckey->clist;
			while (pp)
			{
				SendKey((UINT)pp->data);
				pp = pp->next;
				Sleep(100);
			}
			return TRUE;
		}

		p = p->next;
	}

	return FALSE;
}
//========================================================================================================
void SendKey(UINT uKey)
{
	keybd_event(uKey, 0, 0, 0);
	keybd_event(uKey, 0, KEYEVENTF_KEYUP, 0); 
}
//========================================================================================================
void LoadSettings(LPCTSTR lpFile)
{
	TCHAR szPath[MAX_PATH] = { 0 };

	HANDLE hProcess = INVALID_HANDLE_VALUE;
	DWORD dwTID;
	BOOL fAutoEnd;

	g_usingSLN	= GetPrivateProfileInt(_T("App"), _T("Using"), 1, lpFile);
	g_fTrayOnly = GetPrivateProfileInt(_T("App"), _T("TrayOnly"), 0, lpFile);
	g_fHideWnd	= GetPrivateProfileInt(_T("App"), _T("HideWnd"), 0, lpFile);
	fAutoEnd	= GetPrivateProfileInt(_T("App"), _T("AutoEnd"), 0, lpFile);

	GetPrivateProfileString(_T("App"), _T("Path"), _T(""), szPath, MAX_PATH, lpFile);
	SetDlgItemText(g_hDlgSett, IDC_EDIT_PATH, szPath);

	if (GetPrivateProfileInt(_T("App"), _T("Run"), 0, lpFile) == 1)
	{
		CheckDlgButton(g_hDlgSett, IDC_CHECK_RUN, BST_CHECKED);
		if(GetWar3PID() == 0)	// 防止运行了再运行一次
		{
			hProcess = RunApp(szPath);
		}
	}

	if (fAutoEnd)
		g_hThread = CreateThread(NULL, 0, WaitForApp, (LPVOID)hProcess, 0, &dwTID);
}
//========================================================================================================
void InitTray(HWND hWnd)
{
	g_nid.cbSize = sizeof(NOTIFYICONDATA);
	g_nid.dwInfoFlags = NIIF_INFO;
	g_nid.dwState = NIS_SHAREDICON;
	g_nid.hWnd = hWnd;
	g_nid.uID = IDI_APP;
	g_nid.hIcon = LoadIcon((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		MAKEINTRESOURCE(IDI_APP));
	g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	g_nid.uCallbackMessage = MM_TRAY;

	lstrcpy(g_nid.szTip, _T("wxxKey"));
}
//========================================================================================================
void ShowTray(BOOL bShow)
{
	if (bShow){
		Shell_NotifyIcon(NIM_ADD, &g_nid);
	}else{
		Shell_NotifyIcon(NIM_DELETE, &g_nid);
	}
}

//========================================================================================================
static DWORD WINAPI WaitForApp(LPVOID lParam)
{
	HANDLE hProcess = (HANDLE)lParam;

	// 等到魔兽运行为止
	if (INVALID_HANDLE_VALUE == hProcess)
	{
		while (1)
		{
			DWORD dwPID = GetWar3PID();
			if (dwPID > 0)
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
				break;
			}
			Sleep(100);
		}
	}

	WaitForSingleObject(hProcess, INFINITE);

	CloseHandle(hProcess);

	SendMessage(g_hwndMain, WM_CLOSE, 0, 0);

	return 0;
}