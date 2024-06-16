#pragma once
#include "../inc/Aw.h"
#include "../inc/Cpu.h"

#include "../rai/rai.h"

int		valorantDmaFromTemu(PSHARED_DATA kData);
int		ReportInstrumentedFunction(PKEXCEPTION_RECORD Exception);
PVOID	FindVgkBase(PSHARED_DATA kData);

#define VGK_CLONED_CR3_OFFSET   0x834D0
#define VGK_POOL_OFFSET         0x84CE0

#define DRIVER_STOP		0x0
#define DRIVER_DUMP		0x2
#define DRIVER_QUIT		0xe
#define DRIVER_HALT		0xf

typedef struct _UM_SHARED_DATA {
    INT64 Mode;		// 0x01 = DRIVER_DUMP
    INT64 FreeIndex;
    INT64 Online;
    INT64 SustainedWrites;
} UM_SHARED_DATA;

typedef struct _MAP_DATA {
    PMDL SharedMdl;
    PMDL DumpMdl;
    UM_SHARED_DATA* shared_data;
    PCHAR mem;
} MAP_DATA, *PMAP_DATA;

int UnmapProgramMemory(PMAP_DATA Map) {
    MmUnmapLockedPages(Map->shared_data, Map->SharedMdl);
    MmUnmapLockedPages(Map->mem, Map->DumpMdl);
    MmUnlockPages(Map->SharedMdl);
    MmUnlockPages(Map->DumpMdl);
    IoFreeMdl(Map->SharedMdl);
    IoFreeMdl(Map->DumpMdl);
    return 0;
}

// maybe i can run both of these on the cores instead of as dpcs
// idk idc
// this is also dumb
int MapProgramMemory(PMAP_DATA Map, PCHAR Process) {
    ULONG64 SystemCr3 = __readcr3();
    ULONG64 ProgramCr3 = 0;

    ProgramCr3 = *(PULONG64)(Process + 0x28);
    __writecr3(ProgramCr3);

    Map->SharedMdl = IoAllocateMdl((PVOID)0xb0ba0000, 0x1000, 0, 0, 0);
    Map->DumpMdl = IoAllocateMdl((PVOID)0xdead10cc000, 0x200000, 0, 0, 0);

    MmProbeAndLockPages(Map->SharedMdl, KernelMode, IoReadAccess);
    MmProbeAndLockPages(Map->DumpMdl, KernelMode, IoReadAccess);

    Map->shared_data = MmMapLockedPagesSpecifyCache(Map->SharedMdl, KernelMode, MmNonCached, 0, 0, 0);
    Map->mem = MmMapLockedPagesSpecifyCache(Map->DumpMdl, KernelMode, MmNonCached, 0, 0, 0);

    __writecr3(SystemCr3);
    return 0;
}

int valorantDmaFromTemu(PSHARED_DATA kData) {
    MAP_DATA map = { 0 };
    REMOTE_CALL rcall = { 0 };
    PCHAR Process = (PCHAR)PsInitialSystemProcess;

    while (strcmp("raiden.exe", (Process + 0x5a8)) != 0) {
        Process = (PCHAR)(*(PINT64)(Process + 0x448));
        Process = Process - 0x448;
    }
	
	// stall execution for 2 seconds, so our um application has a chance to initialise everything
    KiStallProcessor(2000000000);

    rcall.Complete = 0;
    rcall.ArgCount = 2;
    rcall.Arguments[0] = (INT64)(&map);
    rcall.Arguments[1] = (INT64)Process;
    rcall.Target = (RCALLTARGET)MapProgramMemory;
    KiExecuteRemoteProcedure(&rcall);

    if (map.shared_data == 0 || map.mem == 0) {
        return 0;
    }

    map.shared_data->Online = 1;

    // get vanguard specific data
    PCHAR VgkBase = FindVgkBase(kData);
    INT64 ClonedCr3 = *(PINT64)(VgkBase + VGK_CLONED_CR3_OFFSET);
    INT64 FreeIndex = *(PINT64)(VgkBase + VGK_CLONED_CR3_OFFSET + 0x8);
    INT64 VgkPool = *(PINT64)(VgkBase + VGK_POOL_OFFSET);

    if (VgkBase == 0) {
        rcall.ArgCount = 1;
        rcall.Arguments[0] = (INT64)(&map);
        rcall.Target = (RCALLTARGET)UnmapProgramMemory;
        KiExecuteRemoteProcedure(&rcall);
        return 0;
    }

    while (ClonedCr3 == 0) {
        ClonedCr3 = *(PINT64)(VgkBase + VGK_CLONED_CR3_OFFSET);
        _mm_pause();
    }

    while (FreeIndex == 0) {
        FreeIndex = *(PINT64)(VgkBase + VGK_CLONED_CR3_OFFSET + 0x8);
        _mm_pause();
    }

    while (VgkPool == 0) {
        VgkPool = *(PINT64)(VgkBase + VGK_POOL_OFFSET);
        _mm_pause();
    }

    map.shared_data->FreeIndex = FreeIndex;

    while (map.shared_data->Mode != DRIVER_DUMP) {
        if (map.shared_data->Mode == DRIVER_HALT) {
            __halt();
        }
        _mm_pause();
    }

    for (;;) {
        memcpy(map.mem, (PVOID)VgkPool, 0x200000);
        map.shared_data->SustainedWrites++;
        if (map.shared_data->Mode != DRIVER_DUMP) {
            rcall.Complete = 0;
            rcall.ArgCount = 1;
            rcall.Arguments[0] = (INT64)(&map);
            rcall.Target = (RCALLTARGET)UnmapProgramMemory;
            KiExecuteRemoteProcedure(&rcall);
            return 0;
        }
    }
}

PVOID FindVgkBase(PSHARED_DATA kData) {
    PVOID* HalPrivateDispatchTable = kData->RtlFindExportedRoutineByName(kData->Ntoskrnl, "HalPrivateDispatchTable");
    PVOID nt_HalpCollectPmcCounters = HalPrivateDispatchTable[0x49];
    PVOID Module = 0;
    PVOID hkHalpCollectPmcCounters = nt_HalpCollectPmcCounters;

    KiSetIllegalDispatch(nt_HalpCollectPmcCounters);

    pRtlPcToFileHeader fnRtlPcToFileHeader = (pRtlPcToFileHeader)kData->RtlFindExportedRoutineByName(kData->Ntoskrnl, "RtlPcToFileHeader");
    fnRtlPcToFileHeader(nt_HalpCollectPmcCounters, &Module);

    if (Module != kData->Ntoskrnl) {
        return 0;
    }

    while (hkHalpCollectPmcCounters == nt_HalpCollectPmcCounters) {
        hkHalpCollectPmcCounters = HalPrivateDispatchTable[0x49];
        _mm_pause();
    }

    KiSetIllegalDispatch(nt_HalpCollectPmcCounters);
    KiRegisterExceptionCallback((EXCEPTION_CALLBACK)ReportInstrumentedFunction, X86_EXCEPT_DEBUG);

    return fnRtlPcToFileHeader(hkHalpCollectPmcCounters, &Module);
}

int ReportInstrumentedFunction(PKEXCEPTION_RECORD Exception) {
    // hooked api!111!
    KiFreezeProcessor();
    UNREFERENCED_PARAMETER(Exception);
    return 0;
}
