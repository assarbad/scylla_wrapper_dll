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
#include "stdafx.h"
#include "scylla_iatfix.h"
#include "ApiReader.h"
#include "ProcessLister.h"
#include "ImportRebuilder.h"
#include "IATSearch.h"

static std::map<DWORD_PTR, ImportModuleThunk> moduleList;

extern "C" SCYLLA_IATFIX_API int scylla_searchIAT(DWORD pid, DWORD_PTR &iatStart, DWORD &iatSize, DWORD_PTR searchStart = 0xDEADBEEF, bool advancedSearch = false)
{
	ApiReader apiReader;
	DWORD_PTR searchAddress = 0;
	DWORD_PTR addressIAT = 0, addressIATAdv = 0;
	DWORD sizeIAT = 0, sizeIATAdv = 0;
	IATSearch iatSearch;
    ProcessLister processLister;

    //need to find correct process by PID
    Process *processPtr = 0;
    std::vector<Process>& processList = processLister.getProcessListSnapshot();
    for(std::vector<Process>::iterator it = processList.begin(); it != processList.end(); ++it) {
        if(it->PID == pid) {
            processPtr = &(*it);
            break;
        }
    }

    if(!processPtr) return SCY_ERROR_PROCOPEN;

    //init process access
	ProcessAccessHelp::closeProcessHandle();
	apiReader.clearAll();

	if (!ProcessAccessHelp::openProcessHandle(processPtr->PID))
	{
		return SCY_ERROR_PROCOPEN;
	}

	ProcessAccessHelp::getProcessModules(processPtr->PID, ProcessAccessHelp::moduleList);

	ProcessAccessHelp::selectedModule = 0;
	ProcessAccessHelp::targetSizeOfImage = ProcessAccessHelp::getSizeOfImageProcess(ProcessAccessHelp::hProcess, ProcessAccessHelp::targetImageBase);
	ProcessAccessHelp::targetImageBase = processPtr->imageBase;

	apiReader.readApisFromModuleList();

    int retVal = SCY_ERROR_IATNOTFOUND; 

    //now actually do some searching
    if(searchStart!=0xDEADBEEF)
	{
		searchAddress = searchStart;
		if (searchAddress)
		{

			if (advancedSearch)
			{
				if (iatSearch.searchImportAddressTableInProcess(searchAddress, &addressIATAdv, &sizeIATAdv, true))
				{
					//Scylla::windowLog.log(L"IAT Search Advanced: IAT VA " PRINTF_DWORD_PTR_FULL L" RVA " PRINTF_DWORD_PTR_FULL L" Size 0x%04X (%d)", addressIATAdv, addressIATAdv - ProcessAccessHelp::targetImageBase, sizeIATAdv, sizeIATAdv);

                    iatStart = addressIATAdv;
                    iatSize = sizeIATAdv;

                    retVal = SCY_ERROR_SUCCESS;
				}
			}


			if (iatSearch.searchImportAddressTableInProcess(searchAddress, &addressIAT, &sizeIAT, false))
			{
				//Scylla::windowLog.log(L"IAT Search Normal: IAT VA " PRINTF_DWORD_PTR_FULL L" RVA " PRINTF_DWORD_PTR_FULL L" Size 0x%04X (%d)", addressIAT, addressIAT - ProcessAccessHelp::targetImageBase, sizeIAT, sizeIAT);

                iatStart = addressIAT;
                iatSize = sizeIAT;

                retVal = SCY_ERROR_SUCCESS;
			}
			
		}
	} else {
        return SCY_ERROR_IATSEARCH;
    }

    processList.clear();
    ProcessAccessHelp::closeProcessHandle();
    apiReader.clearAll();

    return retVal;
}

extern "C" SCYLLA_IATFIX_API int scylla_getImports(DWORD_PTR iatAddr, DWORD iatSize, DWORD pid)
{
    //some things we need
	ApiReader apiReader;
    ProcessLister processLister;

    //need to find correct process by PID
    Process *processPtr = 0;
    std::vector<Process>& processList = processLister.getProcessListSnapshot();
    for(std::vector<Process>::iterator it = processList.begin(); it != processList.end(); ++it) {
        if(it->PID == pid) {
            processPtr = &(*it);
            break;
        }
    }

    if(!processPtr) return SCY_ERROR_PROCOPEN;

    //init process access
	ProcessAccessHelp::closeProcessHandle();
	apiReader.clearAll();

	if (!ProcessAccessHelp::openProcessHandle(processPtr->PID))
	{
		return SCY_ERROR_PROCOPEN;
	}

	ProcessAccessHelp::getProcessModules(processPtr->PID, ProcessAccessHelp::moduleList);

	ProcessAccessHelp::selectedModule = 0;
	ProcessAccessHelp::targetSizeOfImage = ProcessAccessHelp::getSizeOfImageProcess(ProcessAccessHelp::hProcess, ProcessAccessHelp::targetImageBase);
	ProcessAccessHelp::targetImageBase = processPtr->imageBase;

	apiReader.readApisFromModuleList();

    //parse IAT
    apiReader.readAndParseIAT(iatAddr, iatSize, moduleList);

    return SCY_ERROR_SUCCESS;
}

extern "C" SCYLLA_IATFIX_API bool scylla_importsValid()
{
	std::map<DWORD_PTR, ImportModuleThunk>::iterator it_module;
	std::map<DWORD_PTR, ImportThunk>::iterator it_import;
    bool valid = true;

	it_module = moduleList.begin();
	while (it_module != moduleList.end())
	{
		ImportModuleThunk &moduleThunk = it_module->second;

		it_import = moduleThunk.thunkList.begin();
		while (it_import != moduleThunk.thunkList.end())
		{
			ImportThunk &importThunk = it_import->second;

            if(!importThunk.valid) {
                valid = false;
                break;;
            }
			it_import++;
		}

		it_module++;
	}

    return valid;
}

extern "C" SCYLLA_IATFIX_API int scylla_fixDump(WCHAR* dumpFile, WCHAR* iatFixFile)
{
    WCHAR dumpedFilePath[MAX_PATH];
    WCHAR fixedFilePath[MAX_PATH];

    wcscpy_s(fixedFilePath, iatFixFile);
    wcscpy_s(dumpedFilePath, dumpFile);

    //add IAT section to dump
	ImportRebuilder importRebuild(dumpedFilePath);
    importRebuild.enableOFTSupport();

	if (importRebuild.rebuildImportTable(fixedFilePath, moduleList))
	{
        return SCY_ERROR_SUCCESS;
	}
	else
	{
        return SCY_ERROR_IATWRITE;
	}
}

BOOL DumpProcessW(const WCHAR * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const WCHAR * fileResult)
{
	PeParser * peFile = 0;

	if (fileToDump)
	{
		peFile = new PeParser(fileToDump, true);
	}
	else
	{
		peFile = new PeParser(imagebase, true);
	}

	return peFile->dumpProcess(imagebase, entrypoint, fileResult);
}

extern "C" SCYLLA_IATFIX_API bool scylla_dumpProcessW(DWORD_PTR pid, const WCHAR * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const WCHAR * fileResult)
{
	if (ProcessAccessHelp::openProcessHandle((DWORD)pid))
	{
		return DumpProcessW(fileToDump, imagebase, entrypoint, fileResult);
	}
	else
	{
		return FALSE;
	}	
}

extern "C" SCYLLA_IATFIX_API bool scylla_dumpProcessA(DWORD_PTR pid, const char * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const char * fileResult)
{
	WCHAR fileToDumpW[MAX_PATH];
	WCHAR fileResultW[MAX_PATH];

	if (fileResult == 0)
	{
		return FALSE;
	}

	if (MultiByteToWideChar(CP_ACP, 0, fileResult, -1, fileResultW, _countof(fileResultW)) == 0)
	{
		return FALSE;
	}

	if (fileToDump != 0)
	{
		if (MultiByteToWideChar(CP_ACP, 0, fileToDump, -1, fileToDumpW, _countof(fileToDumpW)) == 0)
		{
			return FALSE;
		}

		return scylla_dumpProcessW(pid, fileToDumpW, imagebase, entrypoint, fileResultW);
	}
	else
	{
		return scylla_dumpProcessW(pid, 0, imagebase, entrypoint, fileResultW);
	}
}
