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
#include <stdlib.h>

#include "list.h"

List* list_alloc()
{
	return (List*)calloc(1, sizeof(List));
}

List* list_append(List *list, void *data)
{
	List *last ;

	List *new_item = list_alloc();
	retv_if_fail(NULL != new_item, list);

	new_item->data = data;
	if (NULL == list)
		return new_item;

	last = list_last(list);
	if (NULL != last)
	{
		last->next = new_item;
		new_item->prev = last;
	}

	return list;
}

List* list_prepend(List *list, void *data)
{
	List *new_item = list_alloc();
	retv_if_fail(NULL != new_item, list);

	new_item->data = data;
	new_item->next = list;

	retv_if_fail(NULL != list, new_item);

	new_item->prev = list->prev;
	if (list->prev)
		list->prev->next = new_item;
	list->prev = new_item;

	return new_item;
}

int	list_length(List *list)
{
	int size = 0;
	List *p = list;

	while(p)
	{
		p = p->next;
		size++;
	}

	return size;
}

void list_free(List *list)
{
	List *p = list;

	while (p)
	{
		List *t = p;
		p = p->next;
		free(t);
	}
}

void list_free_full(List *list, FREEFUNC func)
{
	List *p = list;

	while (p)
	{
		List *t = p;
		p = p->next;

		if (func)
			func(t->data);
		free(t);
	}
}

void* list_nth_data(List *list, INT_PTR n)
{
	List *current;
	int i = 0;

	if (NULL == list || n < 0)
		return NULL;

	current = list;

	while (current && i != n)
	{
		i++;
		current = current->next;
	}

	if (NULL != current)
		return current->data;

	return NULL;
}

void* list_last_data(List *list)
{
	List *current = list;
	List *last = list;

	retv_if_fail(NULL != list, NULL);

	while (current)
	{
		last = current;
		current = current->next;
	}

	if (NULL != last)
		return last->data;

	return NULL;
}

void list_foreach(List *list, USERFUNC func, void *user_data)
{
	ret_if_fail(NULL != func);

	while (list)
	{
		List *next = list->next;
		(*func)(list->data, user_data);
		list = next;
	}
}

List* list_last(List *list)
{
	retv_if_fail(NULL != list, list);

	while (list->next)
		list = list->next;

	return list;
}

List* list_nth(List *list, INT_PTR n)
{
	while ((n-- > 0) && list)
		list = list->next;

	return list;
}

List* list_insert(List *list, void* data, INT_PTR pos)
{
	List *new_item;
	List *nth_list;

	if (pos < 0)
		return list_append(list, data);
	else if(pos == 0)
		return list_prepend(list, data);

	nth_list = list_nth(list, pos);
	if (NULL == nth_list)
		return list_append(list, data);

	new_item = list_alloc();
	new_item->data = data;
	new_item->prev = nth_list->prev;

	if (nth_list->prev)
		nth_list->prev->next = new_item;
	new_item->next = nth_list;
	nth_list->prev = new_item;

	if (list == nth_list)
		return new_item;

	return list;
}

List* list_reverse(List *list)
{
	List *last;

	while (list)
	{
		last = list;
		list = last->next;
		last->next = last->prev;
		last->prev = list;
	}

	return last;
}

List* list_find(List *list, void *data)
{
	while (list)
	{
		if (list->data == data)
			break;
		list = list->next;
	}

	return list;
}

List* list_find_custom(List *list, void *data, CMPFUNC func)
{
	retv_if_fail(NULL != func, list);

	while (list)
	{
		if (func(list->data, data) == 0)
			return list;
		list = list->next;
	}

	return NULL;
}

INT_PTR list_position(List *list, List *link)
{
	INT_PTR i = 0;

	while (list)
	{
		if (list == link)
			return i;
		i++;
		list = list->next;
	}

	return -1;
}

INT_PTR list_index(List *list, void *data)
{
	INT_PTR i = 0;

	while (list)
	{
		if (list->data == data)
			return i;
		i++;
		list = list->next;
	}

	return -1;
}

List* list_remove(List *list, void *data)
{
	List *tmp = list;

	while (tmp)
	{
		if (tmp->data != data)
			tmp = tmp->next;
		else
		{
			if (tmp->prev)
				tmp->prev->next = tmp->next;
			if (tmp->next)
				tmp->next->prev = tmp->prev;

			if (list == tmp)
				list = list->next;

			free(tmp);

			break;
		}
	}

	return list;
}

List* list_remove_all(List *list, void *data)
{
	List *tmp = list;

	while (tmp)
	{
		if (tmp->data != data)
			tmp = tmp->next;
		else
		{
			List *next = tmp->next;

			if (tmp->prev)
				tmp->prev->next = next;
			else
				list = next;
			if (next)
				next->prev = tmp->prev;

			free(tmp);
			tmp = next;
		}
	}

	return list;
}

List* list_remove_link(List *list, List *link)
{
	if (link)
	{
		if (link->prev)
			link->prev->next = link->next;
		if (link->next)
			link->next->prev = link->prev;

		if (link == list)
			list = list->next;

		link->next = NULL;
		link->prev = NULL;
	}

	return list;
}

List* list_delete_link(List *list, List *link)
{
	list = list_remove_link (list, link);
	free(link);

	return list;
}