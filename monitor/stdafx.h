/*
MFILEMON - print to file with automatic filename assignment
Copyright (C) 2007-2023 Lorenzo Monti

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#ifdef __GNUC__
#include <sdkddkver.h>
#include "..\common\sec_api.h"
#endif

#include <windows.h>
#include <wchar.h>
#include <string.h>
#include <winspool.h>
#include <winsplp.h>
#include <crtdbg.h>

#define LENGTHOF(x) (sizeof(x)/sizeof((x)[0]))

extern PMONITORINIT g_pMonitorInit;
