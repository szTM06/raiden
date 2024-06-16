#pragma once
#include <ntddk.h>
#include "ntstructs.h"
#include <intrin.h>

#include "Cpu.h"
#include "../rai/rai.h"

//#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

#define BIOS_EXTENDED_BASE	((uint64_t)0xe0000)
#define BIOS_EXTENDED_SIZE	((SIZE_T)((0xfffff + 1)) - BIOS_EXTENDED_BASE)

#define AWI_MEMORY_START	    0x0a000
#define AWI_MEMORY_END		    0x10000

#define AWI_ADDRESS_BASE 	    AWI_MEMORY_START
#define AWI_DATA_BASE_OFFSET    0x2000

typedef struct _SHARED_DATA {
    PVOID   Ntoskrnl;
    pRtlFindExportedRoutineByName   RtlFindExportedRoutineByName;
    PVOID   Self;
    UINT64  KernelCr3;
    UINT64  Kpcrb;
    PVOID   Entrypoint;
    volatile INT64* CpuRunningFlag;
    PVOID   CpuStackBase;
} SHARED_DATA, * PSHARED_DATA;

NTSTATUS			AwiSendGlobalNMI();
NTSTATUS			AwiSendInterrupt(PINTERRUPT Interrupt, int DisableInterrupts);
PLAPIC				AwiMapLapic();
NTSTATUS			AwiStartProcessor(int CpuApic, CPU_BOOT_CONTEXT CpuContext, PCHAR CpuMappedRam);
NTSTATUS			AwiMmAllocateMemory(PVOID* OutAddr, PMDL* OutMdl);
NTSTATUS			AwiSetupSharedData(PSHARED_DATA Data);

NTSTATUS			AwStartProcessor(PCPU_APIC WakeableCPU, CPU_BOOT_CONTEXT CpuContext, PCHAR CpuMappedRam);
NTSTATUS			AwAllocateAwxMemory(PVOID* OutAddr, PMDL* OutMdl);
NTSTATUS			AwSetupSharedData(PSHARED_DATA Data);

// external imports

void				AwxWriteIcr(PLAPIC, INT64, INT64);
void				AwxReadIcr(PLAPIC, PUINT32, PUINT32);

NTSYSAPI PVOID		RtlPcToFileHeader(PVOID, PVOID*);