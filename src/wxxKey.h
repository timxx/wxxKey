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
#pragma once
//========================================================================================================
#ifndef __WXXKEY_H__
#define __WXXKEY_H__
//========================================================================================================
#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>

#include "resource.h"
#include "list.h"
#include "common.h"
//========================================================================================================

const TCHAR szClsName[]	 = TEXT("wxxKey");
const TCHAR szWndTitle[] = TEXT("wxxKey");

HHOOK g_hhook = NULL;

HWND g_hwndMain		= NULL;

HWND g_hwndTab		= NULL;
HWND g_hDlgWarkey	= NULL;
HWND g_hDlgSett		= NULL;
HWND g_hDlgAbout	= NULL;

List*	g_listWarkey  = NULL;
TCHAR	g_szIniFile[MAX_PATH] = {0};

UINT_PTR	g_usingSLN = 0;
BOOL	g_fActived = TRUE;

BOOL	g_fHideWnd = FALSE;
BOOL	g_fTrayOnly = FALSE;

WarVer	g_warVer = vUnset;	/* ħ�ް汾 */

NOTIFYICONDATA	g_nid = { 0 };

HANDLE g_hThread = INVALID_HANDLE_VALUE;
//========================================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

ATOM Register(HINSTANCE hInst);
BOOL InitWindow(HINSTANCE hInst);
void CenterWnd(HWND hwndTo, HWND hwndParent);

void	OnClose(HWND hWnd);
BOOL	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
LRESULT OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr);
void	OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);
void	OnSysCommand(HWND hWnd, UINT cmd, int x, int y);
void	OnTray(HWND hWnd, WPARAM wParam, LPARAM lParam);
void	OnWaitApp(HWND hWnd);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL HookIt(PKBDLLHOOKSTRUCT pkb);

void SendKey(UINT uKey);

void LoadSettings(LPCTSTR lpFile);

void InitTray(HWND hWnd);
void ShowTray(BOOL bShow);
//========================================================================================================
#endif /* __WXXKEY_H__ */
//========================================================================================================