#include "init.h"

// *** Initialise processor

// setup interrupt and exception handling
NTSTATUS KiInitialiseProcessor() {
	PKIDTENTRY64 Idt = ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x1000, 'enoN');
	PINT64 Gdt = ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x60, 'enoN');
	PKPROCESSOR Kprocessor = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KPROCESSOR), 'enoN');

	if (Idt == 0 || Gdt == 0 || Kprocessor == 0) {
		KxBugCheckDispatch(STATUS_INSUFFICIENT_RESOURCES);
	}

	// setup gdt with clone of bootstrap gdt, same as actual nt gdt i think
	Gdt[0] = (INT64)0;
	Gdt[1] = (INT64)0;
	Gdt[2] = (INT64)0x00209b0000000000;
	Gdt[3] = (INT64)0x0040930000000000;
	Gdt[4] = (INT64)0x00cffb000000ffff;
	Gdt[5] = (INT64)0x00cff3000000ffff;

	KIDTENTRY64 IdtEntry = { 0 };
	DTR Dtr = { 0 };
	INT64 Isr = 0;

	IdtEntry.c.b.Selector = 0x10;
	IdtEntry.c.b.a.Present = 1;
	IdtEntry.c.b.a.Type = 0xe;

	for (int i = 0xff; i > -1; i--) {
		Isr = ((INT64)KxRollingIsr + (INT64)((0xff - i) * 9));
		IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
		IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
		IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
		Idt[i] = IdtEntry;
	}

	Isr = (INT64)KxDivisionFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_DIVERR] = IdtEntry;

	Isr = (INT64)KxDebugTrapOrFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_DEBUG] = IdtEntry;

	Isr = (INT64)KxNmiInterrupt;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_NMI] = IdtEntry;

	Isr = (INT64)KxBreakpointTrap;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_BREAKPOINT] = IdtEntry;

	Isr = (INT64)KxOverflowTrap;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_OVERFLOW] = IdtEntry;

	Isr = (INT64)KxBoundRangeFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_BOUND] = IdtEntry;

	Isr = (INT64)KxInvalidOpcodeFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_INVALID_OPCODE] = IdtEntry;

	Isr = (INT64)KxDeviceNotAvailable;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_DEVICE_NOT_AVAILABLE] = IdtEntry;

	Isr = (INT64)KxDoubleFaultAbort;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_DOUBLE_FAULT] = IdtEntry;

	Isr = (INT64)KxCoprocessorFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_COPROCESSOR] = IdtEntry;

	Isr = (INT64)KxUnusedTrap0;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_INVALID_TSS] = IdtEntry;

	Isr = (INT64)KxSegmentFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_SEGMENT_NOT_PRESENT] = IdtEntry;

	Isr = (INT64)KxStackSegmentFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_STACK_FAULT] = IdtEntry;

	Isr = (INT64)KxGeneralProtectionFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_GENERAL_PROTECTION] = IdtEntry;

	Isr = (INT64)KxPageFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_PAGE_FAULT] = IdtEntry;

	Isr = (INT64)KxReservedInterrupt0;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_RESERVED] = IdtEntry;

	Isr = (INT64)KxFpuFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_FLOATING_POINT] = IdtEntry;

	Isr = (INT64)KxAlignmentFault;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_ALIGNMENT_CHECK] = IdtEntry;

	Isr = (INT64)KxMachineCheckAbort;
	IdtEntry.c.b.OffsetLow = (Isr & 0xffff);
	IdtEntry.c.b.OffsetMiddle = (Isr & 0xffff0000) >> 16;
	IdtEntry.c.b.OffsetHigh = (Isr & 0xffffffff00000000) >> 32;
	Idt[X86_EXCEPT_MACHINE_CHECK] = IdtEntry;

	memset(IntCallbacks, 0, sizeof(IntCallbacks));
	memset(ExcCallbacks, 0, sizeof(ExcCallbacks));
	memset(StopCallbacks, 0, sizeof(StopCallbacks));
	BugCheckCallback = 0;

	Dtr.Length = 0xfff;
	Dtr.Base = Idt;
	__lidt(&Dtr);

	Dtr.Length = 0x30;
	Dtr.Base = Gdt;
	_lgdt(&Dtr);

	__writecr8(0x2ULL);
	__writemsr(0xc0000102, (UINT64)Kprocessor);

	Kprocessor->Magic = 0xb0bacafeb0bacafe;
	Kprocessor->Self = Kprocessor;
	Kprocessor->ApicId = CpuxGetCurrentApicId();

	BugCheckInProgress = 0;

	KiRegisterExceptionCallback((EXCEPTION_CALLBACK)KiDoubleFaultAbort, X86_EXCEPT_DOUBLE_FAULT);
	KiRegisterExceptionCallback((EXCEPTION_CALLBACK)KiDbgBreakpointContinue, X86_EXCEPT_BREAKPOINT);
	KiRegisterStopCallback((STOP_CALLBACK)KiStopPageFault, X86_EXCEPT_PAGE_FAULT);

	return STATUS_SUCCESS;
}

NORETURN void KiReleaseProcessor() {
	PKPROCESSOR Kprocessor = (PKPROCESSOR)__readmsr(0xc0000102);
	DTR Idt = { 0 };
	DTR Gdt = { 0 };

	__sidt(&Idt);
	_sgdt(&Gdt);

	if (Idt.Base != 0) {
		ExFreePool(Idt.Base);
	}

	if (Gdt.Base != 0) {
		ExFreePool(Gdt.Base);
	}

	if (Kprocessor != 0) {
		ExFreePool(Kprocessor);
	}

	for (;;) {
		_disable();
		__halt();
	}
}