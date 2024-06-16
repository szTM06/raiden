#include "interrupt.h"

// *** EXCEPTION AND INTERRUPT DISPATCH

void KiDispatchInterrupt(PKINTERRUPT_RECORD Interrupt) {
	__writegsqword(offsetof(KPROCESSOR, LastInterruptTime), KiGetSystemTime());
	KiSwapGs();

	if (Interrupt->Vector < 0x1f) {
		KiDispatchException((PKEXCEPTION_RECORD)Interrupt);
		return;
	}

	if (Interrupt->Vector != 0xffff) {
		Interrupt->Vector = X86_EXCEPT_NMI;
		KiDispatchException((PKEXCEPTION_RECORD)Interrupt);
		return;
	}

	if (IntCallbacks[Interrupt->IntVector] != 0) {
		IntCallbacks[Interrupt->IntVector](Interrupt);
	}
}

void KiDispatchException(PKEXCEPTION_RECORD Exception) {
	if (ExcCallbacks[Exception->FaultCode] != 0) {
		if (ExcCallbacks[Exception->FaultCode](Exception) != 0) {
			return;
		}
	}

	if (StopCallbacks[Exception->FaultCode] != 0) {
		StopCallbacks[Exception->FaultCode](Exception);
	}

	KiBugCheck(KMODE_EXCEPTION_NOT_HANDLED, Exception->FaultCode, Exception->FaultingRip, (INT64)Exception, Exception->AddressSpace);
}

// register function to be called when interrupt is raised
NTSTATUS KiRegisterInterruptCallback(INTERRUPT_CALLBACK Callback, int IntVector) {
	if (IntVector < 0x1f) {
		return STATUS_INVALID_PARAMETER_2;
	}

	IntCallbacks[IntVector] = Callback;
	return STATUS_SUCCESS;
}

// register function to be called when exception is raised
NTSTATUS KiRegisterExceptionCallback(EXCEPTION_CALLBACK Callback, int IntVector) {
	if (IntVector > 0x1f) {
		return STATUS_INVALID_PARAMETER_2;
	}

	ExcCallbacks[IntVector] = Callback;
	return STATUS_SUCCESS;
}

// register function to be called during post processing of exception, prior to dispatching bugcheck (on processor, not windows)
// eg. #PF callback could put value of cr2 somewhere in the KEXCEPTION_RECORD
NTSTATUS KiRegisterStopCallback(STOP_CALLBACK Callback, int IntVector) {
	if (IntVector > 0x1f) {
		return STATUS_INVALID_PARAMETER_2;
	}

	StopCallbacks[IntVector] = Callback;
	return STATUS_SUCCESS;
}

NTSTATUS KiRegisterBugCheckCallback(BUGCHECK_CALLBACK Callback) {
	BugCheckCallback = Callback;
	return STATUS_SUCCESS;
}

// raise #DB if attempted execute at Address
void KiSetIllegalDispatch(PVOID Address) {
	ULONGLONG Dr7 = __readdr(7);
	__writedr(0, (ULONGLONG)Address);

	Dr7 |= 0x2;
	Dr7 &= ~0x1;
	Dr7 &= ~0xf0000;
	//Dr7 |= 0x2000;	// general detect. can cause some issues

	__writedr(7, Dr7);
}

/// *** PREDEFINED EXCEPTION CALLBACKS

int KiDoubleFaultAbort(PKEXCEPTION_RECORD Exception) {
	KiBugCheck(UNEXPECTED_KERNEL_MODE_TRAP, Exception->FaultCode, Exception->FaultingRip, (INT64)Exception, Exception->AddressSpace);
}

// continue all #BP (int3)
int KiDbgBreakpointContinue(PKEXCEPTION_RECORD Exception) {
	UNREFERENCED_PARAMETER(Exception);
	return 1;
}

/// *** PREDEFINED PREBUGCHECK CALLBACKS

void KiStopPageFault(PKEXCEPTION_RECORD Exception) {
	Exception->FaultCode = PAGE_FAULT_IN_NONPAGED_AREA;
	Exception->ErrorCode = __readcr2();
}
