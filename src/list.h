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
#include <basetsd.h>
//========================================================================================================
#ifndef __LIST_H__
#define __LIST_H__
//========================================================================================================
#ifdef __cplusplus
extern "C"	{
#endif

#define ret_if_fail(exp)	\
	if (!(exp))	return 

#define retv_if_fail(exp, code)	\
	if (!(exp))	return (code)

	typedef struct _List
	{
		void *data;

		struct _List *next;
		struct _List *prev;
	}List;

	typedef void (*FREEFUNC)(void *);
	typedef void (*USERFUNC)(void *, void *);
	typedef int  (*CMPFUNC)(void *, void *);

	List*	list_alloc();
	List*	list_append(List *list, void *data);
	List*	list_prepend(List *list, void *data);
	int		list_length(List *list);
	void	list_free(List *list);
	void	list_free_full(List *list, FREEFUNC func);
	void*	list_nth_data(List *list, INT_PTR n);
	void*	list_last_data(List *list);
	void	list_foreach(List *list, USERFUNC func, void *user_data);
	List*	list_last(List *list);
	List*	list_nth(List *list, INT_PTR n);
	List*	list_insert(List *list, void* data, INT_PTR pos);
	List*	list_reverse(List *list);
	List*	list_find(List *list, void *data);
	List*	list_find_custom(List *list, void *data, CMPFUNC func);
	INT_PTR		list_position(List *list, List *link);
	INT_PTR		list_index(List *list, void *data);
	List*	list_remove(List *list, void *data);
	List*	list_remove_all(List *list, void *data);
	List*	list_remove_link(List *list, List *link);
	List*	list_delete_link(List *list, List *link);

#ifdef __cplusplus
}
#endif 
//========================================================================================================
#endif /* __LIST_H__ */
//========================================================================================================
