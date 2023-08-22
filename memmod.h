/*
 * Memory DLL loading code
 * Version 0.0.4
 *
 * Copyright (c) 2004-2015 by Joachim Bauch / mail@joachim-bauch.de
 * http://www.joachim-bauch.de
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is at https://github.com/fancycode/MemoryModule
 *
 * The Initial Developer of the Original Code is Joachim Bauch.
 *
 * Portions created by Joachim Bauch are Copyright (C) 2004-2015
 * Joachim Bauch. All Rights Reserved.
 *
 * The code was largely trimmed down, and a few functions were added / replaced for usage in dlmerge
 * Justin DeWitt
 */

#ifndef __MEMORY_MODULE_HEADER
#define __MEMORY_MODULE_HEADER

#include <windows.h>

typedef void *HMEMORYMODULE;
typedef void *HCUSTOMMODULE;

#ifdef __cplusplus
extern "C" {
#endif

HMEMORYMODULE MemoryLoadLibrary(const void *, size_t);
void MemoryFreeLibrary(HMEMORYMODULE);
int MemoryCallEntryPoint(HMEMORYMODULE);
int MemoryGetExportCount(HMEMORYMODULE);
FARPROC MemoryGetExportAddr(HMEMORYMODULE, int);
WORD MemoryGetExportIdx(HMEMORYMODULE, int);
LPCSTR MemoryGetExportName(HMEMORYMODULE, int);

#ifdef __cplusplus
}
#endif

#endif  // __MEMORY_MODULE_HEADER
