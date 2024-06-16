#include "bugcheck.h"

// *** STOP FUNCTIONS

// bugcheck with NMI_HARDWARE_FAILURE, last resort dont use lol
// saves PKEXCEPTION_RECORD as first member of KUSER_SHARED_DATA
NORETURN void KxBugCheck(PKEXCEPTION_RECORD BugCheck) {
	PVOID* KUSER_SHARED_DATA = (PVOID*)0xFFFFF78000000000;
	KUSER_SHARED_DATA[0] = BugCheck;
	for (int i = 0; i < APIC_MAX_VALUE; i++) {
		if (CpuMap[i].Running == 1 && CpuMap[i].ValidCpu != 0x20000) {
			INTERRUPT NmiInterrupt = { 0 };
			NmiInterrupt.Low.u.DeliveryMode = 0b100;
			NmiInterrupt.High.u.Destination = CpuMap[i].ApicId;
			AwiSendInterrupt(&NmiInterrupt, 0);
			KiFreezeProcessor();
		}
	}
}

// bugcheck with NMI_HARDWARE_FAILURE, last resort dont use lol
// saves PKEXCEPTION_RECORD as first member of KUSER_SHARED_DATA
// takes a single error code instead of a KEXCEPTION_RECORD 
NORETURN void KxBugCheckDispatch(INT64 Code) {
	KEXCEPTION_RECORD buhgcheck = { 0 };

	buhgcheck.FaultCode = Code;
	buhgcheck.AddressSpace = __readcr3();
	KxBugCheck(&buhgcheck);
}

NORETURN void KxFastFail() {
	KiFreezeProcessor();
}

// bugcheck system the standard way, basically KeBugCheckEx
NORETURN void KiBugCheck(INT64 Code, INT64 Param1, INT64 Param2, INT64 Param3, INT64 Param4) {
	if (_interlockedbittestandset64(&BugCheckInProgress, 1) == 1) {
		KiFreezeProcessor();
	}

	if (BugCheckCallback != 0) {
		BugCheckCallback(Code, Param1, Param2, Param3, Param4);
	}

	REMOTE_CALL rcall = { 0 };
	rcall.ArgCount = 5;
	rcall.Arguments[0] = Code;
	rcall.Arguments[1] = Param1;
	rcall.Arguments[2] = Param2;
	rcall.Arguments[3] = Param3;
	rcall.Arguments[4] = Param4;
	rcall.Target = (RCALLTARGET)KeBugCheckEx;
	rcall.Async = 1;
	KiExecuteRemoteProcedure(&rcall);

	for (;;) {
		__halt();
	}
}