/*
*
* Copyright (c) 2014
*
* cypher <the.cypher@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef SCYLLA_IATFIX_EXPORTS
#define SCYLLA_IATFIX_API __declspec(dllexport)
#else
#define SCYLLA_IATFIX_API __declspec(dllimport)
#endif

const BYTE SCY_ERROR_SUCCESS = 0;
const BYTE SCY_ERROR_PROCOPEN = -1;
const BYTE SCY_ERROR_IATWRITE = -2;
const BYTE SCY_ERROR_IATSEARCH = -3;
const BYTE SCY_ERROR_IATNOTFOUND = -4;

//IAT exports
extern "C" SCYLLA_IATFIX_API int scylla_searchIAT(DWORD pid, DWORD_PTR &iatStart, DWORD &iatSize, DWORD_PTR searchStart, bool advancedSearch);
extern "C" SCYLLA_IATFIX_API int scylla_getImports(DWORD_PTR iatAddr, DWORD iatSize, DWORD pid);
extern "C" SCYLLA_IATFIX_API bool scylla_importsValid();
extern "C" SCYLLA_IATFIX_API int scylla_fixDump(WCHAR* dumpFile, WCHAR* iatFixFile);

//dumper exports
extern "C" SCYLLA_IATFIX_API bool scylla_dumpProcessW(DWORD_PTR pid, const WCHAR * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const WCHAR * fileResult);
extern "C" SCYLLA_IATFIX_API bool scylla_dumpProcessA(DWORD_PTR pid, const char * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const char * fileResult);

//rebuilder exports
extern "C" SCYLLA_IATFIX_API bool scylla_rebuildFileW(const WCHAR * fileToRebuild, BOOL removeDosStub, BOOL updatePeHeaderChecksum, BOOL createBackup);
extern "C" SCYLLA_IATFIX_API bool scylla_rebuildFileA(const char * fileToRebuild, BOOL removeDosStub, BOOL updatePeHeaderChecksum, BOOL createBackup);
