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

extern "C" SCYLLA_IATFIX_API int scylla_iatfix(DWORD_PTR iatAddr, DWORD iatSize, DWORD pid, WCHAR* dumpFile, WCHAR* iatFixFile)
{
    //some things we need
	ApiReader apiReader;
    WCHAR dumpedFilePath[MAX_PATH];
    WCHAR fixedFilePath[MAX_PATH];
    std::map<DWORD_PTR, ImportModuleThunk> moduleList;
    ProcessLister processLister;

    wcscpy_s(fixedFilePath, iatFixFile);
    wcscpy_s(dumpedFilePath, dumpFile);

    //need to find correct process by PID
    Process *processPtr = 0;
    std::vector<Process>& processList = processLister.getProcessListSnapshot();
    for(std::vector<Process>::iterator it = processList.begin(); it != processList.end(); ++it) {
        if(it->PID == pid) processPtr = &(*it);
    }

    if(!processPtr) return SCY_ERROR_PROCOPEN;

    //init process access
	if (ProcessAccessHelp::hProcess != 0)
	{
		ProcessAccessHelp::closeProcessHandle();
		apiReader.clearAll();
	}

	if (!ProcessAccessHelp::openProcessHandle(processPtr->PID))
	{
		return SCY_ERROR_PROCOPEN;
	}

	ProcessAccessHelp::getProcessModules(processPtr->PID, ProcessAccessHelp::moduleList);

	apiReader.readApisFromModuleList();

	ProcessAccessHelp::selectedModule = 0;
	ProcessAccessHelp::targetSizeOfImage = processPtr->imageSize;
	ProcessAccessHelp::targetImageBase = processPtr->imageBase;  //important !!
	ProcessAccessHelp::getSizeOfImageCurrentProcess();
	processPtr->imageSize = (DWORD)ProcessAccessHelp::targetSizeOfImage;
	//processPtr->entryPoint = ProcessAccessHelp::getEntryPointFromFile(processPtr->fullPath);

    //parse IAT
    apiReader.readAndParseIAT(iatAddr, iatSize, moduleList);

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

extern "C" SCYLLA_IATFIX_API int scylla_iatsearch(DWORD pid, DWORD_PTR &iatStart, DWORD &iatSize, DWORD_PTR searchStart = 0xDEADBEEF, bool advancedSearch = false)
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
        if(it->PID == pid) processPtr = &(*it);
    }

    if(!processPtr) return SCY_ERROR_PROCOPEN;

    //init process access
	if (ProcessAccessHelp::hProcess != 0)
	{
		ProcessAccessHelp::closeProcessHandle();
		apiReader.clearAll();
	}

	if (!ProcessAccessHelp::openProcessHandle(processPtr->PID))
	{
		return SCY_ERROR_PROCOPEN;
	}

	ProcessAccessHelp::getProcessModules(processPtr->PID, ProcessAccessHelp::moduleList);

	apiReader.readApisFromModuleList();

	ProcessAccessHelp::selectedModule = 0;
	ProcessAccessHelp::targetSizeOfImage = processPtr->imageSize;
	ProcessAccessHelp::targetImageBase = processPtr->imageBase;  //important !!
	ProcessAccessHelp::getSizeOfImageCurrentProcess();
	processPtr->imageSize = (DWORD)ProcessAccessHelp::targetSizeOfImage;

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

                    return SCY_ERROR_SUCCESS;
				}
				else
				{
					return SCY_ERROR_IATNOTFOUND;
				}
			}


			if (iatSearch.searchImportAddressTableInProcess(searchAddress, &addressIAT, &sizeIAT, false))
			{
				//Scylla::windowLog.log(L"IAT Search Normal: IAT VA " PRINTF_DWORD_PTR_FULL L" RVA " PRINTF_DWORD_PTR_FULL L" Size 0x%04X (%d)", addressIAT, addressIAT - ProcessAccessHelp::targetImageBase, sizeIAT, sizeIAT);

                iatStart = addressIAT;
                iatSize = sizeIAT;

                return SCY_ERROR_SUCCESS;
			}
			else
			{
				return SCY_ERROR_IATNOTFOUND;
			}
			
		}
	} else {
        return SCY_ERROR_IATSEARCH;
    }
}