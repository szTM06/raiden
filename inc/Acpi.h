#pragma once
#include <ntddk.h>
#include "ntstructs.h"
#include <intrin.h>

#include "Cpu.h"

#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

#define BIOS_EXTENDED_BASE ((uint64_t)0xe0000)
#define BIOS_EXTENDED_SIZE ((SIZE_T)((0xfffff + 1)) - BIOS_EXTENDED_BASE)

int					AcpiRootChecksum(PXSDP RootHdr);
int					AcpiTableChecksum(ACPISDTHeader* tableHeader);

NTSTATUS            AwiFindDirectMadt(PACPISDTHeader* Madt);
NTSTATUS			AwiParseMadtForProcessors(PACPISDTHeader Madt);
PHYSICAL_ADDRESS	AwiAcpiFindRootTable(PBIOS_DATA MappedBiosData);

PACPISDTHeader		AwFindTableBySignature(PXSDT RootSDT, char* signature);
PXSDT				AwAcpiFindRootTable();

// exports
NTSTATUS            ZwQuerySystemInformation(INT64, PVOID, ULONG, PULONG);