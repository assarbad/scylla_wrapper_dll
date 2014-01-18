#pragma once

#include <windows.h>
#include <winternl.h>


#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define STATUS_SUCCESS   ((NTSTATUS)0x00000000L)
#define DUPLICATE_SAME_ATTRIBUTES   0x00000004

typedef struct _CLIENT_ID{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;


//
// Memory Information Classes for NtQueryVirtualMemory
//
typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName,
	MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;


typedef struct _MEMORY_WORKING_SET_LIST
{
	ULONG	NumberOfPages;
	ULONG	WorkingSetList[1];
} MEMORY_WORKING_SET_LIST, *PMEMORY_WORKING_SET_LIST;

typedef struct _MEMORY_SECTION_NAME
{
	UNICODE_STRING	SectionFileName;
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;


typedef NTSTATUS (WINAPI *def_NtTerminateProcess)(HANDLE ProcessHandle, NTSTATUS ExitStatus);
typedef NTSTATUS (WINAPI *def_NtQueryObject)(HANDLE Handle,OBJECT_INFORMATION_CLASS ObjectInformationClass,PVOID ObjectInformation,ULONG ObjectInformationLength,PULONG ReturnLength);
typedef NTSTATUS (WINAPI *def_NtDuplicateObject)(HANDLE SourceProcessHandle, HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ACCESS_MASK DesiredAccess, BOOLEAN InheritHandle, ULONG Options ); 
typedef NTSTATUS (WINAPI *def_NtQueryInformationFile)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);
typedef NTSTATUS (WINAPI *def_NtQueryInformationThread)(HANDLE ThreadHandle,THREADINFOCLASS ThreadInformationClass,PVOID ThreadInformation,ULONG ThreadInformationLength,PULONG ReturnLength);
typedef NTSTATUS (WINAPI *def_NtQueryInformationProcess)(HANDLE ProcessHandle,PROCESSINFOCLASS ProcessInformationClass,PVOID ProcessInformation,ULONG ProcessInformationLength,PULONG ReturnLength);
typedef NTSTATUS (WINAPI *def_NtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS (WINAPI *def_NtQueryVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID Buffer, ULONG Length, PULONG ResultLength); 
typedef NTSTATUS (WINAPI *def_NtOpenProcess)(PHANDLE ProcessHandle, ACCESS_MASK AccessMask, PVOID ObjectAttributes, PCLIENT_ID ClientId );
typedef NTSTATUS (WINAPI *def_NtOpenThread)(PHANDLE ThreadHandle,ACCESS_MASK DesiredAccess,POBJECT_ATTRIBUTES ObjectAttributes,PCLIENT_ID ClientId);
typedef NTSTATUS (WINAPI *def_NtResumeThread)(HANDLE ThreadHandle, PULONG SuspendCount);
typedef NTSTATUS (WINAPI *def_NtSetInformationThread)(HANDLE ThreadHandle,THREADINFOCLASS ThreadInformationClass,PVOID ThreadInformation,ULONG ThreadInformationLength);
typedef NTSTATUS (WINAPI *def_NtCreateThreadEx)(PHANDLE hThread,ACCESS_MASK DesiredAccess,LPVOID ObjectAttributes,HANDLE ProcessHandle,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,int CreateFlags,ULONG StackZeroBits,LPVOID SizeOfStackCommit,LPVOID SizeOfStackReserve,LPVOID lpBytesBuffer);
typedef NTSTATUS (WINAPI *def_NtSuspendProcess)(HANDLE ProcessHandle);
typedef NTSTATUS (WINAPI *def_NtResumeProcess)(HANDLE ProcessHandle);

typedef ULONG (WINAPI *def_RtlNtStatusToDosError)(NTSTATUS Status);

//Flags from waliedassar
#define NtCreateThreadExFlagCreateSuspended  0x1
#define NtCreateThreadExFlagSuppressDllMains 0x2
#define NtCreateThreadExFlagHideFromDebugger 0x4

class NativeWinApi {
public:
	
	static def_NtCreateThreadEx NtCreateThreadEx;
	static def_NtDuplicateObject NtDuplicateObject;
	static def_NtOpenProcess NtOpenProcess;
	static def_NtOpenThread NtOpenThread;
	static def_NtQueryObject NtQueryObject;
	static def_NtQueryInformationFile NtQueryInformationFile;
	static def_NtQueryInformationProcess NtQueryInformationProcess;
	static def_NtQueryInformationThread NtQueryInformationThread;
	static def_NtQuerySystemInformation NtQuerySystemInformation;
	static def_NtQueryVirtualMemory NtQueryVirtualMemory;
	static def_NtResumeProcess NtResumeProcess;
	static def_NtResumeThread NtResumeThread;
	static def_NtSetInformationThread NtSetInformationThread;
	static def_NtSuspendProcess NtSuspendProcess;
	static def_NtTerminateProcess NtTerminateProcess;


	static def_RtlNtStatusToDosError RtlNtStatusToDosError;

	static void initialize();

	static PPEB getCurrentProcessEnvironmentBlock();
	static PPEB getProcessEnvironmentBlockAddress(HANDLE processHandle);
};
