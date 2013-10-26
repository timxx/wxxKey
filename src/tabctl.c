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
#include "tabctl.h"
#include <commctrl.h>
//========================================================================================================
HWND TabCtl_Create(HWND hwndParent, int x, int y, int w, int h)
{
	HWND hWnd = CreateWindowEx(0, WC_TABCONTROL,
		_T(""),
		WS_CHILD | WS_VISIBLE | TCS_TOOLTIPS,
		x, y, w, h,
		hwndParent, (HMENU)NULL,
		(HINSTANCE)GetWindowLongPtr(hwndParent, GWLP_HINSTANCE),
		0);

	return hWnd;
}
//========================================================================================================
int TabCtl_Append(HWND hwndTab, LPCTSTR lpTitle)
{
	TCITEM ti = { 0 };
	int idx = 0;

	ti.cchTextMax = lstrlen(lpTitle) * sizeof(TCHAR);
	ti.mask = TCIF_TEXT;
	ti.pszText = (LPTSTR)lpTitle;

	idx = (int) SendMessage(hwndTab, TCM_GETITEMCOUNT, 0, 0);

	return (int) SendMessage(hwndTab, TCM_INSERTITEM, idx, (LPARAM)&ti);
}
//========================================================================================================
int	TabCtl_GetCurSel(HWND hwndTab)
{
	return (int)SendMessage(hwndTab, TCM_GETCURSEL, 0, 0);
}
//========================================================================================================
