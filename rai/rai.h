#pragma once

#include <ntddk.h>
#include <intrin.h>

#include "../inc/Aw.h"

/// *** PROCESSOR INITIALISATION, EXCEPTION AND INTERRUPT HANDLING, AND CALLBACKS

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#define NORETURN __declspec(noreturn)

#define X86_EXCEPT_DIVERR               0
#define X86_EXCEPT_DEBUG                1
#define X86_EXCEPT_NMI                  2
#define X86_EXCEPT_BREAKPOINT           3
#define X86_EXCEPT_OVERFLOW             4
#define X86_EXCEPT_BOUND                5
#define X86_EXCEPT_INVALID_OPCODE       6
#define X86_EXCEPT_DEVICE_NOT_AVAILABLE 7
#define X86_EXCEPT_DOUBLE_FAULT         8
#define X86_EXCEPT_COPROCESSOR          9
#define X86_EXCEPT_INVALID_TSS          10
#define X86_EXCEPT_SEGMENT_NOT_PRESENT  11
#define X86_EXCEPT_STACK_FAULT          12
#define X86_EXCEPT_GENERAL_PROTECTION   13
#define X86_EXCEPT_PAGE_FAULT           14
#define X86_EXCEPT_RESERVED             15
#define X86_EXCEPT_FLOATING_POINT       16
#define X86_EXCEPT_ALIGNMENT_CHECK      17
#define X86_EXCEPT_MACHINE_CHECK        18
#define X86_EXCEPT_SIMD_FP              19
#define X86_EXCEPT_VIRTUALIZATION       20
#define X86_EXCEPT_CONTROL_PROTECTION   21
#define X86_EXCEPT_RESERVED_1           22
#define X86_EXCEPT_RESERVED_2           23
#define X86_EXCEPT_RESERVED_3           24
#define X86_EXCEPT_RESERVED_4           25
#define X86_EXCEPT_RESERVED_5           26
#define X86_EXCEPT_HYPERVISOR_INJECTION 28
#define X86_EXCEPT_VMM_COMMUNICATION    29
#define X86_EXCEPT_SECURITY             30

#define STATUS_CPU_GENERIC_FAULT		(INT64)0xe0f00fff
#define STATUS_CPU_INSTRUMENTATION		(INT64)0xe0f00c01
#define STATUS_CPU_ILLEGAL_DISPATCH		(INT64)0xeaaa0000

/// *** 

typedef struct _KCONTEXT {
    INT64 Rip;  // 0x10
    INT64 Rax;  // 0x18
    INT64 Rbx;  // 0x20
    INT64 Rcx;  // 0x28
    INT64 Rdx;
    INT64 Rsi;
    INT64 Rdi;
    INT64 Rsp;  // 0x48 - (0x38 (debug dump))
    INT64 Rbp;
    INT64 R8;
    INT64 R9;
    INT64 R10;
    INT64 R11;
    INT64 R12;
    INT64 R13;
    INT64 R14;
    INT64 R15;  // 0x90
    INT64 Rflags; // 0x98
} KCONTEXT;

typedef struct _KEXCEPTION_RECORD {
    KCONTEXT Context;   // offset 0x10 KPROCESSOR
    INT64 FaultCode;    // 0xa0
    INT64 ErrorCode;    // 0xa8
    INT64 FaultingRip;  // 0xb0
    INT64 AddressSpace; // 0xb8
    INT64 ErrorCodePresent; // 0xc0
    INT64 Reserved;     // 0xc8
} KEXCEPTION_RECORD, * PKEXCEPTION_RECORD;

typedef struct _KINTERRUPT_RECORD {
    KCONTEXT Context;   // offset 0x10 KPROCESSOR
    INT64 Vector;    // 0xa0
    INT64 ExceptCode;    // 0xa8
    INT64 Rip;  // 0xb0
    INT64 Cr3; // 0xb8
    INT64 Reserved; // 0xc0
    INT64 IntVector;// 0xc8
} KINTERRUPT_RECORD, * PKINTERRUPT_RECORD;

PACK(typedef struct _DTR {
    unsigned short Length;
    void* Base;
}) DTR;

/// *** dumbass KPCR because im too lazy to do interrupts properly
/// !!! LastInterrupt must be at offset 0x10, all the exception handling code assumes as such
/// tier one programming

typedef struct _KPROCESSOR {
    INT64 Magic;
    PVOID Self;
    KINTERRUPT_RECORD LastInterrupt;    // offset 0x10
    INT64 LastInterruptTime;
    int ApicId;
    int FreezeExecution;
} KPROCESSOR, * PKPROCESSOR;

/// *** Callbacks for Interrupts, Exceptions, and Bugchecks

typedef int (*INTERRUPT_CALLBACK)(PKINTERRUPT_RECORD);
typedef int (*EXCEPTION_CALLBACK)(PKEXCEPTION_RECORD);
INTERRUPT_CALLBACK IntCallbacks[0x100];
EXCEPTION_CALLBACK ExcCallbacks[0x20];

typedef void (*STOP_CALLBACK)(PKEXCEPTION_RECORD);
STOP_CALLBACK StopCallbacks[0x20];

typedef void (*BUGCHECK_CALLBACK)(INT64, INT64, INT64, INT64, INT64);
BUGCHECK_CALLBACK BugCheckCallback;

// *** 

NTSTATUS    KiInitialiseProcessor();
void        KiSetIllegalDispatch(PVOID);
void        KiDispatchInterrupt(PKINTERRUPT_RECORD Interrupt);
void        KiDispatchException(PKEXCEPTION_RECORD Exception);

NTSTATUS    KiRegisterInterruptCallback(INTERRUPT_CALLBACK Callback, int IntVector);
NTSTATUS    KiRegisterExceptionCallback(EXCEPTION_CALLBACK Callback, int IntVector);
NTSTATUS    KiRegisterBugCheckCallback(BUGCHECK_CALLBACK Callback);
NTSTATUS    KiRegisterStopCallback(STOP_CALLBACK Callback, int IntVector);

int         KiDoubleFaultAbort(PKEXCEPTION_RECORD Exception);
int         KiDbgBreakpointContinue(PKEXCEPTION_RECORD Exception);
void        KiStopPageFault(PKEXCEPTION_RECORD Exception);

NORETURN void KiBugCheck(INT64 Code, INT64 Param1, INT64 Param2, INT64 Param3, INT64 Param4);
NORETURN void KxBugCheck(PKEXCEPTION_RECORD);
NORETURN void KxBugCheckDispatch(INT64);
NORETURN void KxFastFail();
NORETURN void KiReleaseProcessor();

volatile INT64 BugCheckInProgress;

void KxRollingIsr();
void KxDivisionFault();
void KxDebugTrapOrFault();
void KxNmiInterrupt();
void KxBreakpointTrap();
void KxOverflowTrap();
void KxBoundRangeFault();
void KxInvalidOpcodeFault();
void KxDeviceNotAvailable();
void KxDoubleFaultAbort();
void KxCoprocessorFault();
void KxUnusedTrap0();
void KxSegmentFault();
void KxStackSegmentFault();
void KxGeneralProtectionFault();
void KxPageFault();
void KxReservedInterrupt0();
void KxFpuFault();
void KxAlignmentFault();
void KxMachineCheckAbort();

/// *** NTCALL - call functions on NT cores
/// this is dogshit

typedef int (*RCALLTARGET)(INT64, ...);

typedef struct _REMOTE_CALL {
	int ArgCount;
	RCALLTARGET Target;
	int ReturnValue;
	int Complete;
	int Async;
	INT64 Arguments[10];
} REMOTE_CALL, * PREMOTE_CALL;

int KiCallVarArgFunction(RCALLTARGET, PINT64, int);

void KiRemoteDpcEntry(KDPC*, PREMOTE_CALL, PVOID, PVOID);
void KiRemoteApcEntry(KAPC*, PREMOTE_CALL, PVOID, PVOID);

PREMOTE_CALL KiExecuteRemoteProcedure(PREMOTE_CALL);
PREMOTE_CALL KiExecuteRemoteAllProcessors(PREMOTE_CALL);
PREMOTE_CALL KiExecuteRemoteOnProcessorById(PREMOTE_CALL, int);

/// *** LIBRARY

INT64   KiGetSystemTime();
void    KiStallProcessor(INT64 nanoseconds);
void    KiRebootSystem();

// if you want to freeze "isolated cores"
void    KiFreezeProcessor();
void    KiSwapGs();