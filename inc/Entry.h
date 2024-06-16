#pragma once
#include <ntddk.h>
#include "ntstructs.h"

#include "Acpi.h"

#define BIOS_EXTENDED_BASE ((uint64_t)0xe0000)
#define BIOS_EXTENDED_SIZE ((SIZE_T)((0xfffff + 1)) - BIOS_EXTENDED_BASE)

#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

NTSTATUS	DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);