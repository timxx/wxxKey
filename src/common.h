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
#ifndef __COMMON_H__
#define __COMMON_H__
//========================================================================================================
#include <Windows.h>
#include <tchar.h>
//========================================================================================================
typedef enum _WarVer
{
	vUnset = -1,
	vUnknown,
	v120e,
	v124a,
	v124b,
	v124c,
	v124d,
	v124e,
	v125b,
	v126a

}WarVer;

typedef struct _LANGANDCODEPAGE
{
	WORD wLanguage;
	WORD wCodePage;

}LANGANDCODEPAGE;
//========================================================================================================
void	SetFont(HWND hWnd, LPCTSTR lpFace, int size);

void	ShowPopupMenu(HWND hWnd, UINT uID);

BOOL	GetWar3Path(LPTSTR lpBuffer, int nMax);
HANDLE	RunApp(LPTSTR lpExeFile);
DWORD	GetWar3PID();

BOOL	SelectOneFile(HWND hWnd, LPTSTR lpBuffer, int nMax);

BOOL	IsGameStated();
BOOL	IsChatting();

BOOL	EnableDebugPrivilege();

WarVer	GetWar3Version();
DWORD	GetGameDLLBase(DWORD dwWar3PID);
//========================================================================================================
#endif /* __COMMON_H__ */
//========================================================================================================