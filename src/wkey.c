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
#include "wkey.h"

#include <tchar.h>
//========================================================================================================
static void ParseComboKey(LPCTSTR lpKey, List **list);
static void FreeWKeyFull(void *data);
static void MakeCombineKeyString(LPTSTR lpBuffer, int nMax, List *clist);
//========================================================================================================
BOOL LoadFromFile(LPCTSTR lpFile, List **slist)
{
	int count = 0;
	int digit = 0;
	int i;

	TCHAR *pszSNL = NULL;
	TCHAR *pszCB = NULL;

	if (lpFile == NULL || slist == NULL)
		return FALSE;

	count = GetPrivateProfileInt(_T("App"), _T("SLN"), 0, lpFile);
	if (count == 0)
		return TRUE;

	i = count;
	do 
	{
		digit ++;
		i /= 10;
	} while (i > 0);

	pszSNL = (TCHAR *)calloc(4 + digit, sizeof(TCHAR));
	if (NULL == pszSNL)
		return FALSE;

	pszCB = (TCHAR *)calloc(256, sizeof(TCHAR));
	if (NULL == pszCB)
	{
		free(pszSNL);
		return FALSE;
	}

	for(i=1; i<=count; i++)
	{
		wKey *wkey = (wKey *)calloc(1, sizeof(wKey));
		if (wkey == NULL)
			continue;

		wsprintf(pszSNL, _T("SLN%d"), i);

		if (GetPrivateProfileString(pszSNL, _T("Numpad"), _T(""), wkey->Numpad, 7, lpFile) == 0)
		{
			free(wkey);
			continue;
		}

		if (GetPrivateProfileString(pszSNL, _T("Combine"), _T(""), pszCB, 256, lpFile) > 0)
		{
			ParseComboKey(pszCB, &wkey->Combine);
		}

		*slist = list_append(*slist, wkey);
	}

	free(pszCB);
	free(pszSNL);

	return TRUE;
}
//========================================================================================================
BOOL SaveToFile(LPCTSTR lpFile, wKey *wkey, INT_PTR idx)
{
	TCHAR szTemp[256] = { 0 };
	TCHAR szSLN[20] = { 0 };

	if (lpFile == NULL || wkey == NULL)
		return FALSE;

	wsprintf(szSLN, _T("SLN%d"), idx);

	WritePrivateProfileString(szSLN, _T("Numpad"), wkey->Numpad, lpFile);

	MakeCombineKeyString(szTemp, 256, wkey->Combine);

	WritePrivateProfileString(szSLN, _T("Combine"), szTemp, lpFile);

	return TRUE;
}
//========================================================================================================
static void ParseComboKey(LPCTSTR lpKey, List **list)
{
	const TCHAR *p = lpKey;
	if (NULL == lpKey)
		return ;

	while (*p)
	{
		cKey *ckey = (cKey*)calloc(1, sizeof(cKey));
		if (NULL == ckey)
			break;

		ckey->key = *p;
		/* skip the blank before ':' if any */
		while (*p && *p != _T(':'))
			p++;

		/* invalid combo key */
		if (!*p)
			break;
		p++;

		/* skip the blank chars */
		while(*p && *p == _T(' '))
			p++;

		if (!*p)
			break;

		while( *p && *p != _T('|'))
		{
			ckey->clist = list_append(ckey->clist, (void*)*p);
			p++;
		}

		*list = list_append(*list, ckey);

		if (*p == _T('|'))
			p ++;
	}
}
//========================================================================================================
void FreeWKey(List *list)
{
	list_free_full(list, FreeWKeyFull);
}
//========================================================================================================
static void FreeWKeyFull(void *data)
{
	/* free combine list */
	list_free_full(((wKey*)data)->Combine, FreeCKeyFull);
	free(data);
}
//========================================================================================================
void FreeCKeyFull(void *data)
{
	list_free(((cKey*)data)->clist);
	free(data);
}
//========================================================================================================
static void MakeCombineKeyString(LPTSTR lpBuffer, int nMax, List *clist)
{
	while (clist)
	{
		cKey *ckey = (cKey *) clist->data;
		List *p;

		TCHAR szKey[28];

		wsprintf(szKey, _T("%c:"), ckey->key);

		p = ckey->clist;
		while(p)
		{
			TCHAR szTemp[2] = { 0 };

			szTemp[0] = (TCHAR)p->data;

			_tcscat_s(szKey, 28, szTemp);
			p = p->next;
		}

		_tcscat_s(lpBuffer, nMax, szKey);

		clist = clist->next;
		if (clist)
			_tcscat_s(lpBuffer, nMax, _T("|"));
	}
}
//========================================================================================================
BOOL IsKeyExists(wKey *pwk, TCHAR key)
{
	int i;
	List *p;

	retv_if_fail(pwk != NULL, FALSE);

	for(i=0; i<7; i++)
	{
		if (pwk->Numpad[i] == key)
			return TRUE;
	}

	retv_if_fail(pwk->Combine != NULL, FALSE);

	p = pwk->Combine;

	while (p)
	{
		cKey *ckey = (cKey *)p->data;
		if (ckey->key == key)
			return TRUE;

		p = p->next;
	}

	return FALSE;
}
//========================================================================================================