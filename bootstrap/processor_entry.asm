[BITS 16]

PROCESSOR_ENTRY:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, AWI_STACK_BASE
	xchg bx, bx
	call kernel_entry
	
	; wot the fuck
_HALT_PROCESSOR:
	cli
	hlt
	jmp _HALT_PROCESSOR