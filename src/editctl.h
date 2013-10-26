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
#ifndef __EDIT_CONTROL_H__
#define __EDIT_CONTROL_H__
//========================================================================================================

BOOL Edit_Subclass(HWND hEdit);
BOOL Edit_UnSubclass(HWND hEdit);
void Edit_UnSubclassAll();

#endif /* __EDIT_CONTROL_H__ */
//========================================================================================================