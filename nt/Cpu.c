#include "../inc/Cpu.h"
#include "../inc/Aw.h"
#include "../inc/Acpi.h"

CPU_APIC CpuMap[APIC_MAX_VALUE + 1] = { 0 };

KIPI_BROADCAST_WORKER CpuCallback;

// yeah i love this so much
ULONG_PTR CpuCallback(ULONG_PTR teku) {
    UNREFERENCED_PARAMETER(teku);

    int CpuApicId = CpuxGetCurrentApicId();
    PCPU_APIC Cpu = &CpuMap[CpuApicId];

    if (Cpu->ApicId != CpuApicId) { // so this always is correct?? who the fuck wrote this shit. nvm i query all cores first
        Cpu->ValidCpu |= 0x80000000;
    }

    Cpu->WindowsProcessorNumber = KeGetCurrentProcessorNumber();
    Cpu->Running = 1;
    return 1;
}

// we can see what apic ids are running under the scheduler
// could also use nmis
NTSTATUS CpuQueryAllActiveCores() {
    KeIpiGenericCall(CpuCallback, 0);
    for (int i = 0; i < APIC_MAX_VALUE; i++) {
        if (CpuMap[i].ValidCpu & 0x80000000) {
            return STATUS_CPU_APIC_MISMATCH;
        }
    }

    return STATUS_SUCCESS;
}

// query all cores available on system for apic ids
// tbh this function is kinda dogshit, i should have just used zwquerysysteminformation from the getgo
// but i wanted to figure out how acpi tables worked
NTSTATUS CpuQueryAllCores() {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    INT64 Offset = 0;
    PXSDT Xsdt = AwAcpiFindRootTable(&Offset);
    PACPISDTHeader Madt = AwFindTableBySignature(Xsdt, "APIC");

    if (Madt == 0) {
        AwiFindDirectMadt(&Madt);
        Status = AwiParseMadtForProcessors(Madt);

        if (Xsdt != 0) {
            MmUnmapIoSpace(Xsdt, 0x1000);
        }
        
        if (Madt != 0) {
            Madt = (PACPISDTHeader)((PCHAR)Madt - 0x10);
            ExFreePool(Madt);
        }
    }
    else {
        Status = AwiParseMadtForProcessors(Madt);
        if (Xsdt != 0) {
            MmUnmapIoSpace(Xsdt, 0x1000);
        }

        if (Madt != 0) {
            MmUnmapIoSpace(Madt, 0x1000);
        }
    }

    return Status;
}

// find a core valid for waking
NTSTATUS CpuFindWakeable(PCPU_APIC Cpu, PCPU_APIC aCpuList) {
    for (int i = 0; i < APIC_MAX_VALUE; i++) {
        if (aCpuList[i].Running == 0 && aCpuList[i].ValidCpu == 1) {
            Cpu->ApicId = aCpuList[i].ApicId;
            Cpu->AcpiProcId = aCpuList[i].AcpiProcId;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_CPU_UNAVAILABLE_WAKE;
}