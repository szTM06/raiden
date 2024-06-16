[BITS 64]

; we are mapped in the nt kernel
; so lets jump to ourselves within the ntos address space
; intel / amd (i dont know who) didnt add a jmp and change address space instruction ): (to my knowledge)
; this gets around it without the awkwardness of identity mapping 
; change it anyway and catch the resulting page fault
krnl_setup_nt_context:
	lgdt [GDT_LONGMODE.Pointer]
	push 0
	mov dword [rsp + 0x4], GDT_LONGMODE.NoLimit
	mov dword [rsp], krnl_setup_nt_context.long_no_limit
	retf
	
.long_no_limit: 	; unnecessary, limit ignored in long mode. but its 3am and i am done with x86
	mov rax, [AWI_DATA_BASE + 0x220]
	add rax, ntos_gdt - PROCESSOR_ENTRY
	mov [ntos_gdt.ptr + 0x2], rax
	lgdt [ntos_gdt.ptr]
	
	mov ecx, 0xC0000101
	mov eax, dword [AWI_DATA_BASE + 0x230]
	mov edx, dword [AWI_DATA_BASE + 0x234]
	wrmsr	; yeah this is cool thanks amd 64 bit processor but we still need to write msrs like this
	
	mov rax, ntos_i_entry
	sub rax, AWI_ADDRESS_BASE
	add rax, [AWI_DATA_BASE + 0x220]
	
	xor esi, esi
	xor edi, edi

	mov rdx, rax
	and rdx, 0xFFFF
	or rsi, rdx
	or rsi, 0x100000

	mov rdx, rax
	and rdx, 0xFFFFFFFFFFFF0000
	shl rdx, 32
	or rsi, rdx
	mov rbx, 0x00008e0000000000
	or rsi, rbx

	mov rdx, rax
	shr rdx, 32
	or rdi, rdx

	xor ecx, ecx
	mov rdx, AWI_INTERRUPT_BASE
.loop:
	mov [rdx], rsi
	mov [rdx + 8], rdi
	add rdx, 0x10
	inc ecx
	cmp ecx, 0x20
	jle .loop
	
	mov rax, AWI_INTERRUPT_BASE
	sub rax, AWI_ADDRESS_BASE
	add rax, [AWI_DATA_BASE + 0x220]
	mov [ntos_idt + 0x2], rax
	lidt [ntos_idt]
	
	mov rbp, rsp
	sub rbp, AWI_ADDRESS_BASE
	mov rax, [AWI_DATA_BASE + 0x220]
	add rax, rbp
	mov rsp, rax
	mov r15, [AWI_DATA_BASE + 0x220]

	mov rax, [AWI_DATA_BASE + 0x228]
	xchg bx, bx
	mov cr3, rax
	
	; page fault!!!1111!
	
ntos_i_entry:
	xchg bx, bx
	mov rcx, 0xC0000080
	rdmsr
	or rax, 1 << 11
	wrmsr
	
	; iretq to the entry point, absolutely criminal
	mov rcx, AWI_DATA_BASE + 0x210
	sub rcx, AWI_ADDRESS_BASE
	add rcx, r15
	
	; rax contains entrypoint
	; rcx contains pointer to shared struct
	; we need to push SS:RSP (original RSP) -> RFLAGS -> CS -> RIP
	; thats what the interrupt handler does so thats what we should iretq from
	
	mov rax, ntos_final
	sub rax, AWI_ADDRESS_BASE
	add rax, r15
	
	mov rdx, [rcx + 0x38]
	add rdx, 0x1a000
	push 0
	push rdx	; new rsp
	pushfq
	push 0x10
	push rax
	iretq

ntos_final:
	cli
	lgdt [rsp]	; 0 the interrupt table, i dont want this code to loop
	mov rax, [rcx + 0x28]
	call rax
	hlt
	
ntos_idt:
	dw 0x100
	dq 0

ntos_gdt:
	dq 0
	dq 0
	dq 0x00209b0000000000
	dq 0x0040930000000000
	;dq 0x00cffb000000ffff
	;dq 0x00cff3000000ffff
.ptr:
	dw $ - ntos_gdt - 1
	dq 0
	