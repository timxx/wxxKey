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
#include <Windows.h>
#include <tchar.h>

#include "editctl.h"
#include "list.h"
#include "wkey.h"
//========================================================================================================
typedef struct _ProcMap ProcMap;

struct _ProcMap
{
	HWND	hwnd;
	WNDPROC lpfn;
};

//========================================================================================================
static List*	_proc_list = NULL;
extern List*	g_listWarkey;
extern UINT		g_usingSLN;
//========================================================================================================
static LRESULT CALLBACK Edit_SubProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam);

static WNDPROC GetProc(HWND hWnd, ProcMap **ppm);
//========================================================================================================
BOOL Edit_Subclass(HWND hEdit)
{
	ProcMap *pm = (ProcMap *)calloc(1, sizeof(ProcMap));
	
	retv_if_fail(pm != NULL, FALSE);

	pm->hwnd = hEdit;
	pm->lpfn = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)Edit_SubProc);

	if (pm->lpfn == NULL)
	{
		free(pm);
		return FALSE;
	}

	_proc_list = list_append(_proc_list, pm);

	return TRUE;
}
//========================================================================================================
BOOL Edit_UnSubclass(HWND hEdit)
{
	ProcMap *pm = NULL;
	LONG_PTR result;

	WNDPROC lpfn = GetProc(hEdit, &pm);

	retv_if_fail(lpfn != NULL, FALSE);

	result = SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)lpfn);

	_proc_list = list_remove(_proc_list, pm);
	free(pm);

	return result != 0;
}
//========================================================================================================
void Edit_UnSubclassAll()
{
	List *p = _proc_list;

	while (p)
	{
		ProcMap *pm = (ProcMap *) p->data;

		SetWindowLongPtr(pm->hwnd, GWLP_WNDPROC, (LONG_PTR)pm->lpfn);

		p = p->next;
	}

	list_free_full(_proc_list, free);
}
//========================================================================================================
static LRESULT CALLBACK Edit_SubProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
// 	case WM_LBUTTONDOWN:
// 		CallWindowProc(GetProc(hEdit, NULL), hEdit, uMsg, wParam, lParam);
// 		SendMessage(hEdit, EM_SETSEL, 0, -1);
//  		return 0;

	case WM_CHAR:
// 		if (IsKeyExists((wKey*)list_nth_data(g_listWarkey, g_usingSLN), toupper(wParam)))
// 			return 0;
// 		else
			SetWindowText(hEdit, _T(""));
		break;
	}

	return CallWindowProc(GetProc(hEdit, NULL), hEdit, uMsg, wParam, lParam);
}
//========================================================================================================
static WNDPROC GetProc(HWND hWnd, ProcMap **ppm)
{
	List *p = _proc_list;

	while (p)
	{
		ProcMap *pm = (ProcMap *)p->data;

		if (pm->hwnd == hWnd)
		{
			if (ppm != NULL)
				*ppm = pm;

			return pm->lpfn;
		}

		p = p->next;
	}

	return NULL;
}