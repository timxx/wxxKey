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
#ifndef __WKEY_H__
#define __WKEY_H__
//========================================================================================================
#include <Windows.h>

#include "list.h"
//========================================================================================================
/*
 * sample of ini file 
 *
 * [App]
 * SLN=2
 *
 * [SLN1]
 * Numpad=123456
 * Combine=A:BCD|R:SR
 *
 * [SLN2]
 * Numpad=QWEZXC
 * Combine=E:D|A:QWE
 *
 */

/* key solution */
typedef struct _wKey
{
	TCHAR	Numpad[7];	/* numpad keys */
	List*	Combine;	/* combination keys */

}wKey;

/* combination key */
typedef struct _cKey
{
	TCHAR	key;
	List*	clist;
}cKey;
//========================================================================================================
BOOL LoadFromFile(LPCTSTR lpFile, List **slist);
BOOL SaveToFile(LPCTSTR lpFile, wKey *wkey, INT_PTR idx);

void FreeWKey(List *list);
void FreeCKeyFull(void *data);

BOOL IsKeyExists(wKey *pwk, TCHAR key);	/* 判断某个方案是否已经存在某个键 */
//========================================================================================================
#endif
//========================================================================================================
