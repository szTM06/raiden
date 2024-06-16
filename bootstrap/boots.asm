[map all bstrp.map]

PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0

GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

%define AWI_ADDRESS_BASE 	0xa000

%define AWI_INTERRUPT_BASE	AWI_ADDRESS_BASE + 0x1000
%define AWI_STACK_BASE		AWI_ADDRESS_BASE + 0x1300
%define AWI_DATA_BASE		AWI_ADDRESS_BASE + 0x2000
%define AWI_SHARED_BASE		AWI_DATA_BASE

org AWI_ADDRESS_BASE

%include "processor_entry.asm"
%include "boots_entry.asm"
%include "bstrp_nt.asm"