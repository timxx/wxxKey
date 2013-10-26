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
#include "dlgproc.h"
#include "resource.h"
#include "list.h"
#include "wkey.h"
#include "mm.h"
#include "common.h"
#include "editctl.h"
//========================================================================================================
extern List* g_listWarkey;
extern UINT_PTR	g_usingSLN;
extern BOOL g_fHideWnd;
extern BOOL g_fTrayOnly;
extern TCHAR g_szIniFile[MAX_PATH];

extern HANDLE g_hThread;

static WNDPROC _lpfnPrvLBProc = NULL;
//========================================================================================================
static void DoWarkeyCommand(HWND hWnd, int id, int notify, HWND hwndCtl);
static void ShowSolution(HWND hWnd);
static void ClearContent(HWND hWnd);

static LRESULT CALLBACK LBSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//========================================================================================================
INT_PTR CALLBACK DlgWarkeyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hEdit;
			int i;

			for(i=IDC_EDIT_W7; i<=IDC_EDIT_CO; i++)
			{
				hEdit = GetDlgItem(hDlg, i);
				SendMessage(hEdit, EM_LIMITTEXT, 1, 0);

				Edit_Subclass(GetDlgItem(hDlg, i));
			}

			SendDlgItemMessage(hDlg, IDC_EDIT_CM, EM_LIMITTEXT, 26, 0);

			_lpfnPrvLBProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, \
				IDC_LIST_CKEY), GWLP_WNDPROC, (LONG_PTR)LBSubProc);

		}
		return TRUE;

	case WM_COMMAND:
		DoWarkeyCommand(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_CLOSE:

		Edit_UnSubclassAll();

		DestroyWindow(hDlg);
		break;

	case MM_SHOWSLN:
		ShowSolution(hDlg);
		break;
	}

	return FALSE;
}
//========================================================================================================
INT_PTR CALLBACK DlgSettingsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			if (id == IDC_CHECK_TRAY)
			{
				g_fTrayOnly = !g_fTrayOnly;

				WritePrivateProfileString(_T("App"), _T("TrayOnly"),
					g_fTrayOnly ? _T("1") : _T("0"), g_szIniFile);
			}
			else if(id == IDC_CHECK_HIDE)
			{
				g_fHideWnd = !g_fHideWnd;
				WritePrivateProfileString(_T("App"), _T("HideWnd"),
					g_fHideWnd ? _T("1") : _T("0"), g_szIniFile);
			}
			else if(id == IDC_CHECK_RUN)
			{
				if (IsDlgButtonChecked(hDlg, id))
				{
					TCHAR szPath[MAX_PATH] = { 0 };
					if (GetWar3Path(szPath, MAX_PATH))
					{
						SetDlgItemText(hDlg, IDC_EDIT_PATH, szPath);
					}

					WritePrivateProfileString(_T("App"), _T("Run"), _T("1") , g_szIniFile);
				}
				else
				{
					WritePrivateProfileString(_T("App"), _T("Run"), _T("0") , g_szIniFile);
				}
			}
			else if(id == IDC_CHECK_KILL_SELF)
			{
				if (IsDlgButtonChecked(hDlg, id))
				{
					WritePrivateProfileString(_T("App"), _T("AutoEnd"), _T("1"), g_szIniFile);
					if (g_hThread == INVALID_HANDLE_VALUE)
						SendMessage(GetParent(hDlg), MM_WAITAPP, 0, 0);
				}
				else
				{
					WritePrivateProfileString(_T("App"), _T("AutoEnd"), _T("0"), g_szIniFile);
					if (g_hThread != INVALID_HANDLE_VALUE)
					{
						TerminateThread(g_hThread, 0);
						CloseHandle(g_hThread);
						g_hThread = INVALID_HANDLE_VALUE;
					}
				}
			}
			else if(id == IDC_BUTTON_BROWSE)
			{
				TCHAR szTemp[MAX_PATH] = { 0 };

				if (SelectOneFile(hDlg, szTemp, MAX_PATH))
				{
					SetDlgItemText(hDlg, IDC_EDIT_PATH, szTemp);
				}
			}
		}
		break;
	}

	return FALSE;
}
//========================================================================================================
INT_PTR CALLBACK DlgAboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT_ABOUT,
			_T("wxxKey v1.0.4 (Apr. 2012)\r\n")
			_T("https://code.google.com/p/wxxkey/\r\n\r\n")
			_T("by Just Fancy(weitianleung@gmail.com)\r\n\r\n")
			_T("为了没有广告而存在~~~\r\n\r\n")

			_T("游戏中可随时按下Home键禁用或启用改键~\r\n")
			_T("使用Page Up、Page Down可切换方案~~\r\n")
			_T("注意不要使用已设置的按键，目前程序不提供自动检测功能！")

			);
		return FALSE;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	}

	return FALSE;
}
//========================================================================================================
static void DoWarkeyCommand(HWND hWnd, int id, int notify, HWND hwndCtl)
{
	switch(id)
	{
	case IDC_COMBO_SLN: /* 方案改变 */
		if (notify == CBN_SELCHANGE)
		{
			INT_PTR idx;
			if(NULL == g_listWarkey)
				break;

			idx = SendDlgItemMessage(hWnd, id, CB_GETCURSEL, 0, 0);
			if (idx != g_usingSLN)
			{
				g_usingSLN = idx;
				ShowSolution(hWnd);
			}
		}
		break;

	case IDC_BUTTON_SAVE: /* 保存方案 */
		{
			TCHAR szKey[20] = {0};
			int i;
			int count;
			wKey *wkOld = NULL;
			wKey *wkey = (wKey*)calloc(1, sizeof(wKey));

			if (NULL == wkey)
			{
				MessageBox(hWnd, _T("无法保存方案！"), _T("出错了"), MB_ICONERROR);
				break;
			}

			for(i=IDC_EDIT_W7; i<=IDC_EDIT_W2; i++)
			{
				GetDlgItemText(hWnd, i, szKey, 2);
				wkey->Numpad[i - IDC_EDIT_W7] = szKey[0];
			}

			count = (int)SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_GETCOUNT, 0, 0);

			for(i=0; i<count; i++)
			{
				TCHAR szTemp[30] = { 0 };
				TCHAR *p = NULL;
				cKey *ckey = NULL;

				if (SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_GETTEXT, i, (LPARAM)szTemp) <= 0)
					continue;

				ckey = (cKey *)calloc(1, sizeof(cKey));
				if (NULL == ckey)
					continue;

				p = szTemp;

				ckey->key = *p;

				p++ ;	/* 跳过'=' */
				p++ ;
				while (*p)
				{
					ckey->clist = list_append(ckey->clist, (void*)*p);
					p++;
				}

				wkey->Combine = list_append(wkey->Combine, (void*)ckey);
			}

			/* 先移除原有的，再插入新的 */
			wkOld = (wKey*)list_nth_data(g_listWarkey, g_usingSLN);
			if (wkOld)
			{
				list_free_full(wkOld->Combine, FreeCKeyFull);
				g_listWarkey = list_remove(g_listWarkey, wkOld);
			}
			g_listWarkey = list_insert(g_listWarkey, wkey, g_usingSLN);

			if (!SaveToFile(g_szIniFile, wkey, g_usingSLN + 1))
			{
				MessageBox(hWnd, _T("无法保存方案到配置文件！"), _T("出错啦~"), MB_ICONERROR);
			}

			wsprintf(szKey, _T("%d"), list_length(g_listWarkey));
			WritePrivateProfileString(_T("App"), _T("SLN"), szKey, g_szIniFile);

			wsprintf(szKey, _T("%d"), g_usingSLN);
			WritePrivateProfileString(_T("App"), _T("Using"), szKey, g_szIniFile);

			if (SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_GETCOUNT, 0, 0) == 0)
			{
				SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_ADDSTRING, 0, (LPARAM)_T("方案1"));
				SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_SETCURSEL, 0, 0);
			}
		}
		break;

	case IDC_BUTTON_NEW: /* 新建方案 */
		{
			INT_PTR count = list_length(g_listWarkey);
			TCHAR szName[28] = {0};
			wKey *wkey = NULL;

			wsprintf(szName, _T("方案%d"), count + 1);

			count = SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_ADDSTRING, 0, (LPARAM)szName);

			if (count == CB_ERR)
			{
				MessageBox(hWnd, _T("无法新建方案！"), _T("出错了"), MB_ICONERROR);
				break;
			}
			
			wkey = (wKey*)calloc(1, sizeof(wKey));
			if (NULL == wkey)
			{
				MessageBox(hWnd, _T("无法新建方案！"), _T("出错了"), MB_ICONERROR);
				
				SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_DELETESTRING, count, 0);
				break;
			}
			
			g_listWarkey = list_append(g_listWarkey, (void*)wkey);

			g_usingSLN = count;

			count = SendDlgItemMessage(hWnd, IDC_COMBO_SLN, CB_SETCURSEL, count, 0);
			ClearContent(hWnd);
		}
		break;

	case IDC_BUTTON_ADD:
		{
			TCHAR szCKey[30] = {0};
			TCHAR szKey[2] = { 0 };
			TCHAR szKey2[28] = {0};

			GetDlgItemText(hWnd, IDC_EDIT_CO, szKey, 2);
			GetDlgItemText(hWnd, IDC_EDIT_CM, szKey2, 28);

			wsprintf(szCKey, _T("%s=%s"), szKey, szKey2);

			SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_ADDSTRING, 0, (LPARAM)szCKey);
			SetDlgItemText(hWnd, IDC_EDIT_CO, _T(""));
			SetDlgItemText(hWnd, IDC_EDIT_CM, _T(""));
		}
		break;

	case IDM_LB_DELETE:
		{
			INT_PTR idx = SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_GETCURSEL, 0, 0);
			if (idx >= 0)
			{
				SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_DELETESTRING, idx, 0);
			}
		}
		break;
	}
}
//========================================================================================================
static void ShowSolution(HWND hWnd)
{
	wKey *wkey = (wKey *)list_nth_data(g_listWarkey, g_usingSLN);
	int id;
	List *list;

	ClearContent(hWnd);

	if (NULL == wkey)
		return ;

	for(id=IDC_EDIT_W7; id<=IDC_EDIT_W2; id++)
	{
		TCHAR szTemp[2] = {0};
		szTemp[0] = wkey->Numpad[id - IDC_EDIT_W7];
		SetDlgItemText(hWnd, id, (LPCTSTR)szTemp);
	}

	list = wkey->Combine;

	while (list)
	{
		cKey *ckey = (cKey*)list->data;
		TCHAR szTemp[256] = { 0 };
		List *p;

		wsprintf(szTemp, _T("%c="), ckey->key);

		p = ckey->clist;
		while (p)
		{
			TCHAR sz[2] = { 0 };
			sz[0] = (TCHAR)p->data;

			lstrcat(szTemp, sz);
			p = p->next;
		}

		SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_ADDSTRING, 0, (LPARAM)szTemp);

		list = list->next;
	}

}
//========================================================================================================
static void ClearContent(HWND hWnd)
{
	int id;

	for(id=IDC_EDIT_W7; id<=IDC_EDIT_CM; id++)
		SetDlgItemText(hWnd, id, _T(""));

	SendDlgItemMessage(hWnd, IDC_LIST_CKEY, LB_RESETCONTENT, 0, 0);
}
//========================================================================================================
static LRESULT CALLBACK LBSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_RBUTTONUP)
	{
		ShowPopupMenu(GetParent(hWnd), IDR_MENU_LB);
	}

	return CallWindowProc(_lpfnPrvLBProc, hWnd, uMsg, wParam, lParam);
}