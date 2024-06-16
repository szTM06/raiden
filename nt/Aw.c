#include "../inc/Aw.h"
#include "../inc/Acpi.h"

#include "../rai/rai.h"

// *** START PROCESSOR
// 
// standard init sipi using legacy apic
NTSTATUS AwiStartProcessor(int CpuApic, CPU_BOOT_CONTEXT CpuContext, PCHAR CpuMappedRam) {
	_disable();
	INTERRUPT Interrupt = { 0 };
	char CpuStart = (char)(CpuContext >> 12);

	// send init to processor
	// the processor might already be in a "wait for sipi" state, but idk
	// this doesnt hurt
	Interrupt.High.u.Destination = CpuApic;
	Interrupt.Low.u.DeliveryMode = 0b101;

	AwiSendInterrupt(&Interrupt, 0);
	KiStallProcessor(10000000);

	// send sipi to processor
	Interrupt.Low.u.DeliveryMode = 0b110;
	Interrupt.Low.u.Vector = CpuStart;

	AwiSendInterrupt(&Interrupt, 0);
	KiStallProcessor(5000000);

	unsigned int CpuStartFlag = *(unsigned int*)(CpuMappedRam + AWI_DATA_BASE_OFFSET + 0x100);
	if (CpuStartFlag != STATUS_CPU_START_SUCCESS) {
		AwiSendInterrupt(&Interrupt, 0);		// if cpu didnt set flag, resend sipi. bus noise? uh cpu just decided fuck it and ignored it? idk but we retry
		_enable();
		KiStallProcessor(200000000);

		CpuStartFlag = *(unsigned int*)(CpuMappedRam + AWI_DATA_BASE_OFFSET + 0x100);
		if (CpuStartFlag != STATUS_CPU_START_SUCCESS) {
			//KeBugCheck(PROCESSOR_START_TIMEOUT);	// nt bugchecks system if it cant start a processor, we aint nt tho we stronger 
			return STATUS_CPU_TIMEOUT_EXCEEDED;	// yeah idk g
		}
	}

	CpuMap[CpuApic].Running = 1;
	CpuMap[CpuApic].ValidCpu = 0x20000;
	return STATUS_CPU_START_SUCCESS;
}


// *** LAPIC AND INTERRUPT 
//
PLAPIC AwiMapLapic() {
	APIC_BASE_MSR ApicMsr = { 0 };
	PHYSICAL_ADDRESS LAPICAddr = { 0 };

	ApicMsr.AsUInt = __readmsr(0x1b);
	LAPICAddr.QuadPart = ApicMsr.u.APICBase * 0x1000;
	return MmMapIoSpace(LAPICAddr, PAGE_SIZE, MmNonCached);
}

// send nmi to all cores excluding self
NTSTATUS AwiSendGlobalNMI() {
	INTERRUPT Interrupt = { 0 };
	Interrupt.Low.u.DeliveryMode = 0b100;
	Interrupt.Low.u.Destination = 0b11;
	return AwiSendInterrupt(&Interrupt, 1);
}

// send interrupt, only works with MMIO lapic
// does x2apic still allow mmio lapic writes or is it just msr
NTSTATUS AwiSendInterrupt(PINTERRUPT Interrupt, int DisableInterrupts) {
	if (DisableInterrupts == 1) {
		_disable();
	}

	PLAPIC Lapic = AwiMapLapic();
	INTERRUPT RealInterrupt = { 0 };

	// sdm says u should read the icr before modifying it
	// if ur gonna do something undocumented and stupid only do one undocumented stupid thing at a time
	AwxReadIcr(Lapic, &RealInterrupt.Low.AsUInt, &RealInterrupt.High.AsUInt);

	RealInterrupt.High.u.Destination = Interrupt->High.u.Destination;
	RealInterrupt.Low.u.Destination = Interrupt->Low.u.Destination;
	RealInterrupt.Low.u.Vector = Interrupt->Low.u.Vector;
	RealInterrupt.Low.u.DeliveryMode = Interrupt->Low.u.DeliveryMode;
	RealInterrupt.Low.u.DestinationMode = Interrupt->Low.u.DestinationMode;
	RealInterrupt.Low.u.TriggerMode = Interrupt->Low.u.TriggerMode;
	RealInterrupt.Low.u.Level = Interrupt->Low.u.Level;
	
	AwxWriteIcr(Lapic, RealInterrupt.Low.AsUInt, RealInterrupt.High.AsUInt);

	// we likely just sent an nmi to nt, calling MmUnmapIoSpace rn probs isnt a good idea, nor necessary lol
	// but by all means try it
	if (BugCheckInProgress) {
		return STATUS_SUCCESS;
	}

	MmUnmapIoSpace(Lapic, PAGE_SIZE);
	if (DisableInterrupts == 1) {
		_enable();
	}

	return STATUS_SUCCESS;
}


// *** MEMORY AND CONTEXT FOR PROCESSOR
//
NTSTATUS AwiMmAllocateMemory(PVOID* OutAddr, PMDL* OutMdl) {
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PHYSICAL_ADDRESS BaseAddress = { 0 };
	PVOID MappedBase = 0;

	BaseAddress.QuadPart = AWI_MEMORY_START;
	MappedBase = MmMapIoSpaceEx(BaseAddress, AWI_MEMORY_END - AWI_MEMORY_START, PAGE_EXECUTE_READWRITE | PAGE_NOCACHE);

	if (MappedBase != 0) {
		Status = STATUS_SUCCESS;
	}

	*OutAddr = MappedBase;
	*OutMdl = 0;
	return Status;
}

NTSTATUS AwiSetupSharedData(PSHARED_DATA Data) {
	UNICODE_STRING strFindExportedRByName = RTL_CONSTANT_STRING(L"RtlFindExportedRoutineByName");
	PVOID fnRtlFindExportedRoutineByName = MmGetSystemRoutineAddress(&strFindExportedRByName);
	PVOID ntoskrnl = 0;
	PINT64 KernelEProcess = (PINT64)PsInitialSystemProcess;

	if (fnRtlFindExportedRoutineByName == 0) {
		return STATUS_UNSUCCESSFUL;
	}

	if (RtlPcToFileHeader(fnRtlFindExportedRoutineByName, &ntoskrnl) == 0) {
		return STATUS_UNSUCCESSFUL;
	}

	Data->Ntoskrnl = ntoskrnl;
	Data->RtlFindExportedRoutineByName = (pRtlFindExportedRoutineByName)fnRtlFindExportedRoutineByName;
	//Data->KernelCr3 = __readcr3();		// TRIPLE FAULT LFG
	Data->KernelCr3 = KernelEProcess[5];	// directory table base for system EPROCESS
	Data->Kpcrb = __readmsr(0xC0000101);
	return STATUS_SUCCESS;
}

/// *** friendly error checking wrappers

NTSTATUS AwAllocateAwxMemory(PVOID* OutAddr, PMDL* OutMdl) {
	if (OutAddr == 0 || OutMdl == 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}
	return AwiMmAllocateMemory(OutAddr, OutMdl);
}

NTSTATUS AwStartProcessor(PCPU_APIC WakeableCPU, CPU_BOOT_CONTEXT CpuContext, PCHAR CpuMappedRam) {
	if (WakeableCPU == 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}
	
	if (WakeableCPU->Running == 1) {
		return STATUS_CPU_ALREADY_RUNNING;
	}

	if (WakeableCPU->ApicId > APIC_MAX_VALUE) {
		return STATUS_CPU_APIC_NOT_VALID;
	}

	if (CpuContext > 0xff00) {
		return STATUS_CPU_CONTEXT_INVALID;
	}

	if (CpuContext & 0xff) {
		return STATUS_CPU_CONTEXT_INVALID;
	}

	return AwiStartProcessor(WakeableCPU->ApicId, CpuContext, CpuMappedRam);
}

NTSTATUS AwSetupSharedData(PSHARED_DATA Data) {
	if (Data == 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}

	return AwiSetupSharedData(Data);
}