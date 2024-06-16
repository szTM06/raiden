struc gdt_entry_struct

	limit_low:   resb 2
	base_low:	resb 2
	base_middle: resb 1
	access:	  resb 1
	granularity: resb 1
	base_high:   resb 1

endstruc

[BITS 16]

kernel_entry:	; ok not really a "kernel" but let me have this
	cli
	xchg bx, bx
	xor eax, eax
	mov ax, ds
	shl eax, 0x4
	add eax, GDT
	mov [gdtr + 2], eax
	mov eax, GDT_end
	sub eax, GDT
	mov [gdtr], ax
	lgdt [gdtr]
	
	mov eax, cr0 
	or al, 1
	mov cr0, eax
	jmp 0x8:kernel_protected_entry
	
kernel_exception_not_handled:
	mov eax, 0xe000002c
	mov dword [AWI_DATA_BASE + 0x100], eax
	jmp _HALT_PROCESSOR

gdtr dw 0
	 dd 0
GDT:
gdt_null_entry:
	istruc gdt_entry_struct
		dq 0
	iend

gdt_kernel_code_entry:
	istruc gdt_entry_struct
		at limit_low,	dw 0xffff
		at base_low, 	dw 0
		at base_middle,	db 0
		at access, 		db 0x9A
		at granularity, db 0x40
		at base_high,	db 0
	iend
GDT_end:

[BITS 32]

; we are in PROTECTED MODE
kernel_protected_entry:
	xchg bx, bx
	mov eax, dword 0xe0000000
	mov dword [AWI_DATA_BASE + 0x100], eax
	; now we have to go into LONG MODE
	; we didnt even have to use memory segmentation in protected mode but im adding it just in case (and because i couldnt do the direct real to long im x86 baby)

	; i copied and pasted this from osdev :3333333
	; meoooowwwww ;3;3;3;3;3;
	; f000 - ptes
	; e000 - thing before pte pde or smth
	; d000 - thing after pml4. pml3? no why would it be that
	; c000 - pml4 & shared base
kernel_setup_longmode:
	mov edi, 0xc000
	mov cr3, edi
	
	; yeah this wont cause a problem on peoples computers
	mov dword [edi], 0xd003
	mov dword [edi + 0x4], 0
	add edi, 0x1000
	mov dword [edi], 0xe003
	mov dword [edi + 0x4], 0
	add edi, 0x1000
	mov dword [edi], 0xf003
	mov dword [edi + 0x4], 0
	add edi, 0x1000
	
	mov ebx, 0x00000003
	mov ecx, 0x200
 
.setEntry:
	mov dword [edi], ebx
	mov dword [edi + 0x4], 0
	add ebx, 0x1000
	add edi, 8
	loop .setEntry
	
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax
	
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr
	
	mov eax, cr0
	and ax, 0xFFFB
	or ax, 0x2
	mov cr0, eax
	
	mov eax, cr4
	or ax, 3 << 9
	mov cr4, eax
	
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax
	
kernel_setup_longmode.skipGDT:
	lgdt [gdt_compatibility.Pointer]
	jmp gdt_compatibility.Code:kernel_long_entry
	
[BITS 64]
 
kernel_long_entry:
	jmp krnl_setup_nt_context
	hlt
	
	
	; this is all probably useless but whatever
gdt_compatibility:
	.Null: equ $ - gdt_compatibility
		dq 0
	.Code: equ $ - gdt_compatibility
		dd 0
		db 0
		db PRESENT | NOT_SYS | EXEC | RW
		db GRAN_4K | LONG_MODE | 0xF
		db 0								 
	.Pointer:
		dw $ - gdt_compatibility - 1
		dq gdt_compatibility
		
GDT_LONGMODE:
	.Null: equ $ - GDT_LONGMODE
		dq 0
		dq 0
	.NoLimit: equ $ - GDT_LONGMODE
		dq 0x00209b0000000000
		dq 0x0040930000000000
	.Pointer:
		dw $ - GDT_LONGMODE - 1
		dq GDT_LONGMODE