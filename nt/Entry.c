#include "../inc/Entry.h"
#include "../inc/Aw.h"
#include "../inc/Cpu.h"

#include "../rai/rai.h"

// thank god for https://wiki.osdev.org/SMP
// os devs u have my heart <3
// 

char Bootstrap[] = { 0xFA, 0x31, 0xC0, 0x8E, 0xD8, 0x8E, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0xB3, 0x87, 0xDB, 0xE8, 0x04, 0x00, 0xFA, 0xF4, 0xEB, 0xFC, 0xFA, 0x87, 0xDB, 0x66, 0x31, 0xC0, 0x8C, 0xD8, 0x66, 0xC1, 0xE0, 0x04, 0x66, 0x05, 0x5E, 0xA0, 0x00, 0x00, 0x66, 0xA3, 0x5A, 0xA0, 0x66, 0xB8, 0x6E, 0xA0, 0x00, 0x00, 0x66, 0x2D, 0x5E, 0xA0, 0x00, 0x00, 0xA3, 0x58, 0xA0, 0x0F, 0x01, 0x16, 0x58, 0xA0, 0x0F, 0x20, 0xC0, 0x0C, 0x01, 0x0F, 0x22, 0xC0, 0xEA, 0x6E, 0xA0, 0x08, 0x00, 0x66, 0xB8, 0x2C, 0x00, 0x00, 0xE0, 0x66, 0xA3, 0x00, 0xC1, 0xEB, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0x40, 0x00, 0x66, 0x87, 0xDB, 0xB8, 0x00, 0x00, 0x00, 0xE0, 0xA3, 0x00, 0xC1, 0x00, 0x00, 0xBF, 0x00, 0xC0, 0x00, 0x00, 0x0F, 0x22, 0xDF, 0xC7, 0x07, 0x03, 0xD0, 0x00, 0x00, 0xC7, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0x81, 0xC7, 0x00, 0x10, 0x00, 0x00, 0xC7, 0x07, 0x03, 0xE0, 0x00, 0x00, 0xC7, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0x81, 0xC7, 0x00, 0x10, 0x00, 0x00, 0xC7, 0x07, 0x03, 0xF0, 0x00, 0x00, 0xC7, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0x81, 0xC7, 0x00, 0x10, 0x00, 0x00, 0xBB, 0x03, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x02, 0x00, 0x00, 0x89, 0x1F, 0xC7, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0x81, 0xC3, 0x00, 0x10, 0x00, 0x00, 0x83, 0xC7, 0x08, 0xE2, 0xEC, 0x0F, 0x20, 0xE0, 0x83, 0xC8, 0x20, 0x0F, 0x22, 0xE0, 0xB9, 0x80, 0x00, 0x00, 0xC0, 0x0F, 0x32, 0x0D, 0x00, 0x01, 0x00, 0x00, 0x0F, 0x30, 0x0F, 0x20, 0xC0, 0x66, 0x83, 0xE0, 0xFB, 0x66, 0x83, 0xC8, 0x02, 0x0F, 0x22, 0xC0, 0x0F, 0x20, 0xE0, 0x66, 0x0D, 0x00, 0x06, 0x0F, 0x22, 0xE0, 0x0F, 0x20, 0xC0, 0x0D, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x22, 0xC0, 0x0F, 0x01, 0x15, 0x35, 0xA1, 0x00, 0x00, 0xEA, 0x22, 0xA1, 0x00, 0x00, 0x08, 0x00, 0xEB, 0x45, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0xAF, 0x00, 0x0F, 0x00, 0x25, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x40, 0x00, 0x1F, 0x00, 0x3F, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x01, 0x14, 0x25, 0x5F, 0xA1, 0x00, 0x00, 0x6A, 0x00, 0xC7, 0x44, 0x24, 0x04, 0x10, 0x00, 0x00, 0x00, 0xC7, 0x04, 0x24, 0x83, 0xA1, 0x00, 0x00, 0xCB, 0x48, 0x8B, 0x04, 0x25, 0x20, 0xC2, 0x00, 0x00, 0x48, 0x05, 0xD6, 0x02, 0x00, 0x00, 0x48, 0x89, 0x04, 0x25, 0xF8, 0xA2, 0x00, 0x00, 0x0F, 0x01, 0x14, 0x25, 0xF6, 0xA2, 0x00, 0x00, 0xB9, 0x01, 0x01, 0x00, 0xC0, 0x8B, 0x04, 0x25, 0x30, 0xC2, 0x00, 0x00, 0x8B, 0x14, 0x25, 0x34, 0xC2, 0x00, 0x00, 0x0F, 0x30, 0x48, 0xB8, 0x78, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x2D, 0x00, 0xA0, 0x00, 0x00, 0x48, 0x03, 0x04, 0x25, 0x20, 0xC2, 0x00, 0x00, 0x31, 0xF6, 0x31, 0xFF, 0x48, 0x89, 0xC2, 0x48, 0x81, 0xE2, 0xFF, 0xFF, 0x00, 0x00, 0x48, 0x09, 0xD6, 0x48, 0x81, 0xCE, 0x00, 0x00, 0x10, 0x00, 0x48, 0x89, 0xC2, 0x48, 0x81, 0xE2, 0x00, 0x00, 0xFF, 0xFF, 0x48, 0xC1, 0xE2, 0x20, 0x48, 0x09, 0xD6, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8E, 0x00, 0x00, 0x48, 0x09, 0xDE, 0x48, 0x89, 0xC2, 0x48, 0xC1, 0xEA, 0x20, 0x48, 0x09, 0xD7, 0x31, 0xC9, 0xBA, 0x00, 0xB0, 0x00, 0x00, 0x48, 0x89, 0x32, 0x48, 0x89, 0x7A, 0x08, 0x48, 0x83, 0xC2, 0x10, 0xFF, 0xC1, 0x83, 0xF9, 0x20, 0x7E, 0xEE, 0xB8, 0x00, 0xB0, 0x00, 0x00, 0x48, 0x2D, 0x00, 0xA0, 0x00, 0x00, 0x48, 0x03, 0x04, 0x25, 0x20, 0xC2, 0x00, 0x00, 0x48, 0x89, 0x04, 0x25, 0xCE, 0xA2, 0x00, 0x00, 0x0F, 0x01, 0x1C, 0x25, 0xCC, 0xA2, 0x00, 0x00, 0x48, 0x89, 0xE5, 0x48, 0x81, 0xED, 0x00, 0xA0, 0x00, 0x00, 0x48, 0x8B, 0x04, 0x25, 0x20, 0xC2, 0x00, 0x00, 0x48, 0x01, 0xE8, 0x48, 0x89, 0xC4, 0x4C, 0x8B, 0x3C, 0x25, 0x20, 0xC2, 0x00, 0x00, 0x48, 0x8B, 0x04, 0x25, 0x28, 0xC2, 0x00, 0x00, 0x66, 0x87, 0xDB, 0x0F, 0x22, 0xD8, 0x66, 0x87, 0xDB, 0xB9, 0x80, 0x00, 0x00, 0xC0, 0x0F, 0x32, 0x48, 0x0D, 0x00, 0x08, 0x00, 0x00, 0x0F, 0x30, 0xB9, 0x10, 0xC2, 0x00, 0x00, 0x48, 0x81, 0xE9, 0x00, 0xA0, 0x00, 0x00, 0x4C, 0x01, 0xF9, 0x48, 0xB8, 0xC0, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x2D, 0x00, 0xA0, 0x00, 0x00, 0x4C, 0x01, 0xF8, 0x48, 0x8B, 0x51, 0x38, 0x48, 0x81, 0xC2, 0x00, 0xA0, 0x01, 0x00, 0x6A, 0x00, 0x52, 0x9C, 0x6A, 0x10, 0x50, 0x48, 0xCF, 0xFA, 0x0F, 0x01, 0x14, 0x24, 0x48, 0x8B, 0x41, 0x28, 0xFF, 0xD0, 0xF4, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x40, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void KiProcessorEntry(PSHARED_DATA);
void ProcessorEntry(PSHARED_DATA);

NTSTATUS DriverEntry(PDRIVER_OBJECT edog, PUNICODE_STRING xeuler) {
    UNREFERENCED_PARAMETER(xeuler);
    UNREFERENCED_PARAMETER(edog);

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PCHAR MappedIo = 0;
    PMDL Mdl = 0;
    CPU_APIC Cpu = { 0 };
    SHARED_DATA Data = { 0 };
    volatile INT64 CpuRunningFlag = 0;

    PCHAR OldLowMm = ExAllocatePool2(POOL_FLAG_NON_PAGED, AWI_MEMORY_END - AWI_MEMORY_START, 'enoN');
    if (OldLowMm == 0) {
        return Status;
    }

    Status = CpuQueryAllCores();
    if (Status != STATUS_SUCCESS) {
        ExFreePool(OldLowMm);
        return Status;
    }

    Status = CpuQueryAllActiveCores();
    if (Status != STATUS_SUCCESS) {
        ExFreePool(OldLowMm);
        return Status;
    }

    Status = CpuFindWakeable(&Cpu, CpuMap);
    if (Status != STATUS_SUCCESS) {
        ExFreePool(OldLowMm);
        return Status;
    }

    AwAllocateAwxMemory(&MappedIo, &Mdl);
    AwSetupSharedData(&Data);
    Data.Self = MappedIo;
    Data.Entrypoint = (PVOID)KiProcessorEntry;
    Data.CpuRunningFlag = &CpuRunningFlag;
    Data.CpuStackBase = ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x20000, 'enoN');

    if (Data.CpuStackBase == 0) {
        ExFreePool(OldLowMm);
        return STATUS_CPU_CONTEXT_INVALID;
    }

    memcpy(OldLowMm, MappedIo, AWI_MEMORY_END - AWI_MEMORY_START);
    memcpy(MappedIo, Bootstrap, sizeof(Bootstrap));
    memcpy(MappedIo + AWI_DATA_BASE_OFFSET + 0x210, &Data, sizeof(SHARED_DATA));

    Status = AwStartProcessor(&Cpu, AWI_MEMORY_START, MappedIo);
    if (Status == STATUS_CPU_START_SUCCESS) {
        int CpuNtosQueryAmt = 0;
        while (*(Data.CpuRunningFlag) != CPU_START_OK) {    // query cpu long mode nt flag

            if (*(Data.CpuRunningFlag) == CPU_START_OK_NO_DEALLOCATE) {
                Status = STATUS_SUCCESS;
                break;
            }

            KiStallProcessor(5000000);
            if (CpuNtosQueryAmt > 600) {
                Status = STATUS_CPU_EXCEPTION_UNHANDLED;    // cpu probably faulted or halted. if it faulted you probably arent even going to survive to this line
                                                            // or YOU didnt set the cpu running flag ):
                break;
            }
            CpuNtosQueryAmt++;
        }
    }

    memcpy(MappedIo, OldLowMm, AWI_MEMORY_END - AWI_MEMORY_START);
    MmUnmapIoSpace(MappedIo, AWI_MEMORY_END - AWI_MEMORY_START);
    ExFreePool(OldLowMm);
    return Status;
}

// Entrypoint of started processor. Controlled by Entrypoint member of shared data struct
// Setting CpuRunningFlag to CPU_START_OK will trigger immediate cleanup of the bootstrap code.
// !! If you need members of the Shared Data structure, copy them !!
//
// Entrypoint is called with pointer to Shared Data struct, gs register points to KPCR for the processor that executed AwiSetupSharedData (specifically the rdmsr)
// ((so assume it is garbage, and could cause issues))
// sse is enabled, avx is disabled, nx is enabled, lstar zero, assume idt and gdt are zero
// kernel functions which are safe to call at IRQL >= DISPATCH_LEVEL mostly work. most kernel apis work (ie Mm Ob), but not all of them are tested
// at this point any fault will kill it and your whole system, so dont write dumb code
//
void KiProcessorEntry(PSHARED_DATA Data) {
    SHARED_DATA kData = { 0 };
    memcpy(&kData, Data, sizeof(SHARED_DATA));
    _mm_mfence();
    *(Data->CpuRunningFlag) = CPU_START_OK;

    // enable exception handling and interrupt handling
    KiInitialiseProcessor();

    // enable caching and write thru
    ULONG64 Cr0 = __readcr0();
    Cr0 &= ~((1ULL << 30) | (1ULL << 29));
    __writecr0(Cr0);

    // call the "real" entry point
    ProcessorEntry(&kData);

    // free processor specific shit and halt
    KiReleaseProcessor();
}

void valorantDmaFromTemu(PSHARED_DATA);
PSHARED_DATA gData;

void fastfailredirect(PKINTERRUPT_RECORD intr) {
    intr->Context.Rcx = (INT64)gData;
    intr->Context.Rip = (INT64)valorantDmaFromTemu;
    intr->Context.Rsp = intr->Context.Rsp + 0x28;
}

// *** PROCESSOR ENTRY POINT ***
//
void ProcessorEntry(PSHARED_DATA Data) {
    gData = Data;
    KiRegisterInterruptCallback((INTERRUPT_CALLBACK)fastfailredirect, 0x29);
    __fastfail(0xaa11);
}