/*
MFILEMON - print to file with automatic filename assignment
Copyright (C) 2007-2021 Lorenzo Monti

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

#include "stdafx.h"
#include "defs.h"

LPCWSTR szMonitorName = L"mfilemon";
LPCWSTR szDescription = L"Multi file port";
LPCWSTR szAppTitle = L"Multi file port monitor";
LPCWSTR szTrue = L"true";
LPCWSTR szFalse = L"false";

#if (!defined(MFILEMONLANG) || MFILEMONLANG == 0x0409)

LPCWSTR szMsgUserCommandLocksSpooler = L"User command is locking the spooler. Wait anyway?";
#ifdef MFILEMONUI
LPCWSTR szMsgInvalidPortName = L"Insert a valid port name (no backslashes allowed).";
LPCWSTR szMsgBrowseFolderTitle = L"Output folder:";
LPCWSTR szMsgProvideFileName = L"Insert a valid pattern.";
LPCWSTR szMsgInvalidFileName = L"A pattern cannot contain the following characters: /:*?\"<>\r\n"
                               L"except * and ? can be present in a \"search field\".";
LPCWSTR szMsgNoAddOnRemoteSvr = L"Unable to add a port on a remote server.";
LPCWSTR szMsgPortExists = L"A port with this name already exists.";
LPCWSTR szMsgNoConfigOnRemoteSvr = L"Unable to configure a port on a remote server.";
LPCWSTR szMsgNoDropOnRemoteSvr = L"Unable to drop a port on a remote server.";
LPCWSTR szMsgBadInteger = L"Insert a valid number.";
#endif

#elif (MFILEMONLANG == 0x0410)

LPCWSTR szMsgUserCommandLocksSpooler = L"Il comando utente sta bloccando lo spooler. Attendere?";
#ifdef MFILEMONUI
LPCWSTR szMsgInvalidPortName = L"Inserire un nome di porta valido (non sono ammessi backslash).";
LPCWSTR szMsgBrowseFolderTitle = L"Directory di output:";
LPCWSTR szMsgProvideFileName = L"Inserire un pattern valido.";
LPCWSTR szMsgInvalidFileName = L"Un pattern non può contenere i seguenti caratteri: /:*?\"<>\r\n"
                               L"con l'eccezione di * e ? che possono comparire in un \"campo di ricerca\"";
LPCWSTR szMsgNoAddOnRemoteSvr = L"Impossibile aggiungere una porta su un server remoto.";
LPCWSTR szMsgPortExists = L"Esiste già una porta con questo nome.";
LPCWSTR szMsgNoConfigOnRemoteSvr = L"Impossibile configurare una porta su un server remoto.";
LPCWSTR szMsgNoDropOnRemoteSvr = L"Impossibile eliminare una porta su un server remoto.";
LPCWSTR szMsgBadInteger = L"Inserire un numero valido.";
#endif

#endif