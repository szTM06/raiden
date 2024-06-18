#include "ntcall.h"

/// ***	this code is dogshit, i wanna rewrite a lot of it but im lazy
///		there is no reason REMOTE_CALL struct shouldnt be automatically created by KiExecuteRemoteOnProcessorById
///		varargs exist for a reason...

void KiRemoteDpcEntry(KDPC* Dpc, PREMOTE_CALL rcall, PVOID SystemArgument1, PVOID SystemArgument2) {
	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
	rcall->ReturnValue = KiCallVarArgFunction(rcall->Target, rcall->Arguments, rcall->ArgCount);
	rcall->Complete = 1;
}

void KiRemoteApcEntry(KAPC* Apc, PREMOTE_CALL rcall, PVOID SystemArgument1, PVOID SystemArgument2) {
	UNREFERENCED_PARAMETER(Apc);
	UNREFERENCED_PARAMETER(rcall);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
}

/// erm what the sigma
/// this is actually criminal, i have a function that does this but im too lazy to import it
/// i wrote this in 5 minutes
int KiCallVarArgFunction(RCALLTARGET target, PINT64 ArgArray, int ArgAmt) {
	switch (ArgAmt) {
	case 0:
		return -1;
	case 1:
		return target(ArgArray[0]);
	case 2:
		return target(ArgArray[0], ArgArray[1]);
	case 3:
		return target(ArgArray[0], ArgArray[1], ArgArray[2]);
	case 4:
		return target(ArgArray[0], ArgArray[1], ArgArray[2], ArgArray[3]);
	case 5:
		return target(ArgArray[0], ArgArray[1], ArgArray[2], ArgArray[3], ArgArray[4]);
	case 6:
		return target(ArgArray[0], ArgArray[1], ArgArray[2], ArgArray[3], ArgArray[4], ArgArray[5]);
	case 7:
		return target(ArgArray[0], ArgArray[1], ArgArray[2], ArgArray[3], ArgArray[4], ArgArray[5], ArgArray[6]);
	case 8:
		return target(ArgArray[0], ArgArray[1], ArgArray[2], ArgArray[3], ArgArray[4], ArgArray[5], ArgArray[6], ArgArray[7]);
	default:
		return -1;
	}
}

PREMOTE_CALL KiExecuteRemoteProcedure(PREMOTE_CALL rcall) {
	for (int i = 0; i < APIC_MAX_VALUE; i++) {
		if (CpuMap[i].Running == 1 && CpuMap[i].ValidCpu != 0x20000) {
			return KiExecuteRemoteOnProcessorById(rcall, CpuMap[i].WindowsProcessorNumber);
		}
	}

	return 0;
}

PREMOTE_CALL KiExecuteRemoteAllProcessors(PREMOTE_CALL rcall) {
	for (int i = 0; i < APIC_MAX_VALUE; i++) {
		if (CpuMap[i].Running == 1 && CpuMap[i].ValidCpu != 0x20000) {
			KiExecuteRemoteOnProcessorById(rcall, CpuMap[i].WindowsProcessorNumber);
		}
	}

	return 0;
}

PREMOTE_CALL KiExecuteRemoteOnProcessorById(PREMOTE_CALL rcall, int WindowsProcessorNumber) {
	KDPC Dpc = { 0 };
	KeInitializeDpc(&Dpc, KiRemoteDpcEntry, rcall);
	KeSetTargetProcessorDpc(&Dpc, (CCHAR)WindowsProcessorNumber);
	if (KeInsertQueueDpc(&Dpc, (PVOID)1, (PVOID)2) != 1) {
		// KiBugCheck uses this function, if we cant queue dpcs then calling KiBugCheck is recursive
		KxBugCheckDispatch(0xe3);
	}

	if (rcall->Async == 0) {
		while (rcall->Complete == 0) {
			_mm_pause();
		}
	}

	return rcall;
}
