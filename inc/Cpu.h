#pragma once
#include <ntddk.h>
#include "ntstructs.h"

#define APIC_MAX_VALUE 0xff
#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

#define CPU_START_OK                         (INT64)0x2
#define CPU_START_OK_NO_DEALLOCATE           (INT64)0x4
#define CPU_CONTINUE_RUNNING                 (INT64)0xc

#define STATUS_CPU_START_SUCCESS             (NTSTATUS)0xe0000000
#define STATUS_CPU_TIMEOUT_EXCEEDED          (NTSTATUS)0xe0000001
#define STATUS_CPU_APIC_NOT_VALID            (NTSTATUS)0xe0000002
#define STATUS_CPU_ALREADY_RUNNING           (NTSTATUS)0xe0000003
#define STATUS_CPU_CONTEXT_INVALID           (NTSTATUS)0xe0000004
#define STATUS_CPU_UNAVAILABLE_WAKE          (NTSTATUS)0xe0000005
#define STATUS_CPU_APIC_MISMATCH             (NTSTATUS)0xe000000c

#define STATUS_CPU_EARLY_FAILURE             (NTSTATUS)0xe000001d
#define STATUS_CPU_EXCEPTION_UNHANDLED       (NTSTATUS)0xe000002c

#define STATUS_AWI_INVALID_PARAMETER         (NTSTATUS)0xe000000d
#define STATUS_AWI_X2APIC_UNSUPPORTED        (NTSTATUS)0xe000000e

typedef PACPISDTHeader (*pHalAcpiGetTable)(PINT64 Reserved, INT64 Signature);

typedef INT64 CPU_BOOT_CONTEXT;

typedef struct _CPU_APIC_MAPPING {
	int AcpiProcId;
	int ApicId;
	int WindowsProcessorNumber;
	int Running;
	int ValidCpu;
} CPU_APIC, * PCPU_APIC;

CPU_APIC CpuMap[APIC_MAX_VALUE + 1];		// this is terrible but intel does some stupid shit with their apic ids
											// another w for team red

int				CpuxGetCurrentApicId(void);

NTSTATUS		CpuQueryAllActiveCores();
NTSTATUS		CpuQueryAllCores();
NTSTATUS		CpuFindWakeable(PCPU_APIC Cpu, PCPU_APIC CpuMap);
