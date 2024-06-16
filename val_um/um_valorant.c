#define _CRT_SECURE_NO_WARNINGS
#include "Windows.h"
#include "stdio.h"

#define DRIVER_STOP		0x0
#define DRIVER_DUMP		0x2
#define DRIVER_HALT		0xf
#define DRIVER_QUIT		0xe

typedef struct _UM_SHARED_DATA {
	INT64 Mode;		// 0x01 = DRIVER_DUMP
	INT64 FreeIndex;
	INT64 Online;
	INT64 SustainedWrites;
} SHARED_DATA;

// yeah nice
BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
	puts("[i] closing");
	SHARED_DATA* data = (PVOID)0xb0ba0000ULL;
	data->Mode = DRIVER_HALT;
	for (;;) {
		// once you set driver halt it immediately starts cleaning up
	}
	
	return FALSE;
}

int main() {
	// main entry point
	SHARED_DATA* data = VirtualAlloc((PVOID)0xb0ba0000ULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (data == 0) {
		puts("error allocating shared data");
		return 1;
	}

	if (SetConsoleCtrlHandler(CtrlHandler, 1) == 0) {
		puts("[!] error setting exit handler");
		return -1;
	}

	VirtualLock(data, 0x1000);
	puts("allocated shared data :3");
	// we cant resolve #PF at dpc level
	memset(data, 0xcc, 0x1000);
	memset(data, 0, sizeof(SHARED_DATA));
	data->Mode = DRIVER_STOP;

	PVOID GuardedMmCopy = (PVOID)0xdead10cc000ULL;
	if (VirtualAlloc(GuardedMmCopy, 0x200000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE | PAGE_NOCACHE) == 0) {
		int err = GetLastError();
		puts("error allocating guarded region");
		return 1;
	}

	VirtualLock(GuardedMmCopy, 0x200000);
	// we cant resolve #PF at dpc level lol
	memset(GuardedMmCopy, 0xcc, 0x200000);
	puts("allocated guarded region >:)");

	while (data->Online == 0) {
		_mm_pause();
	}
	
	puts("comms online");
	while (data->FreeIndex == 0) {
		_mm_pause();
	}

	puts("got freeindex");
	PVOID GuardedRegion = (PVOID)((data->FreeIndex) << 39);

	puts("press enter to start dumping");
	system("pause");

	data->Mode = DRIVER_DUMP;
	puts("dumping");

	for (;;) {
		system("cls");
		printf("sustained writes - %d\n", (int)data->SustainedWrites);
		Sleep(2000);
	}

	return 0;
}