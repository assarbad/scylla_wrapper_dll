```
This is a wrapper around Scylla. 
It exports functions for IAT fixing, dumping and PE rebuilding.

based on http://github.com/NtQuery/Scylla commit 46f9e0eb9e  (v0.9.2)

What has been changed:
- Native API calls (Nt*) replaced by WinAPI calls
- stripped all WTL/ATL dependencies
- stripped GUI (obviously)
```

## Exports ##
    :::c++
    //searches IAT, writes to iatStart, iatSize
    int scylla_searchIAT(DWORD pid, DWORD_PTR &iatStart, DWORD &iatSize, DWORD_PTR searchStart, bool advancedSearch); 
    //reads the imports, iatAddr is VA
    int scylla_getImports(DWORD_PTR iatAddr, DWORD iatSize, DWORD pid, LPVOID invalidImportCallback = NULL);
    //are all imports valid?
    bool scylla_importsValid();
    //cut an Import, because its invalid or whatever reason. Calling this from within the invalidImportCallback will crash! 
    //Call it after scylla_getImports call returned !
    bool scylla_cutImport(DWORD_PTR apiAddr);
    //fix the dump
    int scylla_fixDump(WCHAR* dumpFile, WCHAR* iatFixFile, WCHAR* sectionName = L".scy");
    //fix a mapped dump
    int scylla_fixMappedDump(DWORD_PTR iatVA, DWORD_PTR FileMapVA, HANDLE hFileMap); 
    //get imported DLL count
    int scylla_getModuleCount();
    //get total API Imports count
    int scylla_getImportCount();
    //enumerate imports tree
    void scylla_enumImportTree(LPVOID enumCallBack);
    
    //dumps a process
    bool scylla_dumpProcessW(DWORD_PTR pid, const WCHAR * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const WCHAR * fileResult);
    bool scylla_dumpProcessA(DWORD_PTR pid, const char * fileToDump, DWORD_PTR imagebase, DWORD_PTR entrypoint, const char * fileResult);
    
    //rebuilds a files PE header
    bool scylla_rebuildFileW(const WCHAR * fileToRebuild, BOOL removeDosStub, BOOL updatePeHeaderChecksum, BOOL createBackup);
    bool scylla_rebuildFileA(const char * fileToRebuild, BOOL removeDosStub, BOOL updatePeHeaderChecksum, BOOL createBackup);

## Return Codes ##
const BYTE SCY_ERROR_SUCCESS = 0;
const BYTE SCY_ERROR_PROCOPEN = -1;
const BYTE SCY_ERROR_IATWRITE = -2;
const BYTE SCY_ERROR_IATSEARCH = -3;
const BYTE SCY_ERROR_IATNOTFOUND = -4;

## Usage ##

typedef int (*SEARCHIAT) (DWORD, DWORD_PTR &, DWORD &, DWORD_PTR, bool);
typedef int (*GETIMPORTS) (DWORD_PTR, DWORD, DWORD, LPVOID);
typedef bool (*IMPORTSVALID) ();
typedef int (*FIXDUMP) (WCHAR*, WCHAR*);

HMODULE lib = LoadLibrary(_T("scylla_iatfix"));
SEARCHIAT searchIAT = (SEARCHIAT) GetProcAddress(lib, "scylla_searchIAT");
GETIMPORTS getImports = (GETIMPORTS) GetProcAddress(lib, "scylla_getImports");
IMPORTSVALID importsValid = (IMPORTSVALID) GetProcAddress(lib, "scylla_importsValid");
FIXDUMP fixDump = (FIXDUMP) GetProcAddress(lib, "scylla_fixDump");

DWORD iatStart = 0xDEADBEEF;
DWORD iatSize = 0xDEADBEEF;

int search = searchIAT(fdProcessInfo->dwProcessId, iatStart, iatSize, eip, false);

if(search==0) int imports = getImports(iatStart, iatSize, pid);
bool valid = importsValid();
if(valid) int fix = fixDump(dumpFileName, IatFixFileName);

## Definitions ##

typedef void*(*fCallback)(LPVOID invalidImport);

e.g. void* cbInvalidImport(void* apiAddr)

//pointer on this used in scylla_enumImportTree as argument
typedef struct
{
    bool NewDll;
    int NumberOfImports;
    ULONG_PTR ImageBase;
    ULONG_PTR BaseImportThunk;
    ULONG_PTR ImportThunk;
    char* APIName;
    char* DLLName;
} ImportEnumData

''Calling convention''
The pre-compiled binaries and project standard uses _cdecl calling convention.
For assembly users, this means you have to push arguments from right-to-left onto the stack
and clean the stack yourself after calling.
