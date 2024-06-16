#include "lib.h"

void KiStallProcessor(INT64 nanoseconds) {
	if (nanoseconds == 0) {
		return;
	}

	INT64 clock = KiGetSystemTime();
	INT64 target = clock + (nanoseconds / 100);

	while (clock < target) {
		clock = KiGetSystemTime();
	}
}

INT64 KiGetSystemTime() {	// sipping that oop koolaid
	LARGE_INTEGER clock;
	KeQuerySystemTimePrecise(&clock);
	return clock.QuadPart;
}

void KiRebootSystem() {
	KiBugCheck(POWER_FAILURE_SIMULATE, 0, 0, 0, 0);
	//KiStallProcessor(2000000000);
	//KxFastFail();
}

void KiFreezeProcessor() {
	for (;;) {
		_disable();
		__halt();
	}
}
