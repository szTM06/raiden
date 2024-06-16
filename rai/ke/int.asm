extern KiDispatchInterrupt : proto

.code

; THIS IS SOME OF THE WORST MOST UNMAINTAINABLE CODE EVER CONCEIVED

; void KiDispatchInterrupt( INT64 FaultCode, 
;                           INT64 ErrorCode, 
;                           INT64 FaultingRip, 
;                           PVOID Context );

KxDivisionFault PROC

    push 0
    jmp KxPreprocessInterrupt
    
KxDivisionFault ENDP



KxDebugTrapOrFault PROC

    push 1
    jmp KxPreprocessInterrupt

KxDebugTrapOrFault ENDP



KxNmiInterrupt PROC

    push 2
    jmp KxPreprocessInterrupt
    
KxNmiInterrupt ENDP



KxBreakpointTrap PROC

    push 3
    jmp KxPreprocessInterrupt
    
KxBreakpointTrap ENDP



KxOverflowTrap PROC

    push 4
    jmp KxPreprocessInterrupt
    
KxOverflowTrap ENDP



KxBoundRangeFault PROC

    push 5
    jmp KxPreprocessInterrupt
    
KxBoundRangeFault ENDP



KxInvalidOpcodeFault PROC

    push 6
    jmp KxPreprocessInterrupt
    
KxInvalidOpcodeFault ENDP



KxDeviceNotAvailable PROC

    push 7
    jmp KxPreprocessInterrupt
    
KxDeviceNotAvailable ENDP



KxDoubleFaultAbort PROC

    push 8
    jmp KxPreprocessInterruptCode
    
KxDoubleFaultAbort ENDP



KxCoprocessorFault PROC

    push 9
    jmp KxPreprocessInterrupt
    
KxCoprocessorFault ENDP



KxUnusedTrap0 PROC

    push 10
    jmp KxPreprocessInterruptCode
    
KxUnusedTrap0 ENDP



KxSegmentFault PROC

    push 11
    jmp KxPreprocessInterruptCode
    
KxSegmentFault ENDP



KxStackSegmentFault PROC

    push 12
    jmp KxPreprocessInterruptCode
    
KxStackSegmentFault ENDP



KxGeneralProtectionFault PROC

    push 13
    jmp KxPreprocessInterruptCode

KxGeneralProtectionFault ENDP



KxPageFault PROC

    push 14
    jmp KxPreprocessInterruptCode
    
KxPageFault ENDP



KxReservedInterrupt0 PROC

    hlt
    
KxReservedInterrupt0 ENDP



KxFpuFault PROC

    push 16
    jmp KxPreprocessInterrupt
    
KxFpuFault ENDP



KxAlignmentFault PROC

    push 17
    jmp KxPreprocessInterruptCode
    
KxAlignmentFault ENDP



KxMachineCheckAbort PROC

    push 18
    jmp KxPreprocessInterrupt
    
KxMachineCheckAbort ENDP

; PREPROCESS INTERRUPT FOR DISPATCH WITHOUT CODE



; rsp = vector num
; rsp + 8 = rip
; rsp + 10 = cs
; rsp + 18 = rflags
; rsp + 20 = rsp
; rsp + 28 = ss
KxPreprocessInterrupt PROC

    cmp dword ptr gs:[0], 0B0BACAFEh
    je InterruptPreprocessNoswap
    swapgs

InterruptPreprocessNoswap:

    ; save context
    ; Rip is at gs:10h, and so on and so forth
    mov gs:[018h], rax
    mov gs:[020h], rbx
    mov gs:[028h], rcx
    mov gs:[030h], rdx
    mov gs:[038h], rsi
    mov gs:[040h], rdi

    mov gs:[050h], rbp
    mov gs:[058h], r8
    mov gs:[060h], r9
    mov gs:[068h], r10
    mov gs:[070h], r11
    mov gs:[078h], r12
    mov gs:[080h], r13
    mov gs:[088h], r14
    mov gs:[090h], r15

    ; rsp
    mov rax, [rsp + 020h]
    mov gs:[048h], rax

    ; rflags
    mov rax, [rsp + 018h]
    mov gs:[098h], rax

    ; rip
    mov rax, [rsp + 08h]
    mov gs:[010h], rax

    ; setup KEXCEPTION_RECORD
    mov gs:[0b0h], rax

    mov rax, [rsp]
    mov gs:[0a0h], rax

    mov rax, cr3
    mov gs:[0b8h], rax

    ; interrupt vector for rolling isr
    mov rax, [rsp + 010h]
    shr rax, 010h
    mov gs:[0c8h], rax

    xor eax, eax
    mov al, byte ptr [rsp + 010h]
    mov [rsp + 010h], rax

    xor eax, eax
    mov gs:[0c0h], eax

    mov rcx, gs:[08h]
    add rcx, 010h
    mov rbp, rsp
    sub rsp, 020h
    and rsp, 0fffffffffffffff0h

    ;cmp dword ptr gs:[0], 0B0BACAFEh
    ;jne InterruptPreprocessNoRestore
    ;swapgs

InterruptPreprocessNoRestore:
    call KiDispatchInterrupt
        
    mov rsp, rbp
    jmp KxReturnFromInterrupt

KxPreprocessInterrupt ENDP

; PREPROCESS INTERRUPT FOR DISPATCH WITH CODE
; rsp = vector num
; rsp + 8 = error code
; rsp + 10 = rip
; rsp + 18 = cs
; rsp + 20 = rflags
; rsp + 28 = rsp
; rsp + 30 = ss
KxPreprocessInterruptCode PROC

    cmp dword ptr gs:[0], 0B0BACAFEh
    je InterruptPreprocessCodeNoswap
    swapgs

InterruptPreprocessCodeNoswap:

    ; save context (so we can use the registers)
    ; Rip is at gs:8, and so on and so forth
    mov gs:[018h], rax
    mov gs:[020h], rbx
    mov gs:[028h], rcx
    mov gs:[030h], rdx
    mov gs:[038h], rsi
    mov gs:[040h], rdi

    mov gs:[050h], rbp
    mov gs:[058h], r8
    mov gs:[060h], r9
    mov gs:[068h], r10
    mov gs:[070h], r11
    mov gs:[078h], r12
    mov gs:[080h], r13
    mov gs:[088h], r14
    mov gs:[090h], r15

    mov rax, [rsp + 028h]
    mov gs:[048h], rax

    mov rax, [rsp + 020h]
    mov gs:[098h], rax

    mov rax, [rsp + 010h]
    mov gs:[10h], rax

    ; i hate this
    ; but it is also really fun

    ; setup KEXCEPTION_RECORD
    mov gs:[0b0h], rax

    mov rax, [rsp]
    mov gs:[0a0h], rax

    mov rax, cr3
    mov gs:[0b8h], rax

    mov rax, [rsp + 8]
    mov gs:[0a8h], rax

    mov eax, 01h
    mov gs:[0c0h], rax

    mov rcx, gs:[08h]
    add rcx, 010h
    mov rbp, rsp
    sub rsp, 020h
    and rsp, 0fffffffffffffff0h

    ;cmp dword ptr gs:[0], 0B0BACAFEh
    ;jne InterruptPreprocessCodeNoRestore
    ;swapgs

InterruptPreprocessCodeNoRestore:
    call KiDispatchInterrupt
    
    mov rsp, rbp
    jmp KxReturnFromInterrupt

KxPreprocessInterruptCode ENDP



;
;   INTERRUPT RETURN
;

KxReturnFromInterrupt PROC

    cmp dword ptr gs:[0], 0B0BACAFEh
    je InterruptReturnNoswap
    swapgs

InterruptReturnNoswap:

    mov rax, gs:[0c0h]
    test eax, eax
    jnz InterruptReturnWithCode

    ; rsp
    mov rax, gs:[048h]
    mov [rsp + 020h], rax

    ; rflags
    mov rax, gs:[098h]
    mov [rsp + 018h], rax

    ; rip
    mov rax, gs:[010h]
    mov [rsp + 08h], rax
    jmp InterruptReturnFromInterrupt

InterruptReturnWithCode:

    mov rax, gs:[048h]
    mov [rsp + 028h], rax

    mov rax, gs:[098h]
    mov [rsp + 020h], rax

    mov rax, gs:[10h]
    mov [rsp + 010h], rax

    jmp InterruptReturnFromInterruptCode

InterruptReturnFromInterrupt:

    mov rax, gs:[018h]
    mov rbx, gs:[020h]
    mov rcx, gs:[028h]
    mov rdx, gs:[030h]
    mov rsi, gs:[038h]
    mov rdi, gs:[040h]

    mov rbp, gs:[050h]
    mov r8, gs:[058h]
    mov r9, gs:[060h]
    mov r10, gs:[068h]
    mov r11, gs:[070h]
    mov r12, gs:[078h]
    mov r13, gs:[080h]
    mov r14, gs:[088h]
    mov r15, gs:[090h]

    cmp dword ptr gs:[0], 0B0BACAFEh
    jne InterruptReturnFromInterruptNoRestore
    swapgs

InterruptReturnFromInterruptNoRestore:
    add rsp, 08h
    iretq
    
InterruptReturnFromInterruptCode:

    mov rax, gs:[018h]
    mov rbx, gs:[020h]
    mov rcx, gs:[028h]
    mov rdx, gs:[030h]
    mov rsi, gs:[038h]
    mov rdi, gs:[040h]

    mov rbp, gs:[050h]
    mov r8, gs:[058h]
    mov r9, gs:[060h]
    mov r10, gs:[068h]
    mov r11, gs:[070h]
    mov r12, gs:[078h]
    mov r13, gs:[080h]
    mov r14, gs:[088h]
    mov r15, gs:[090h]

    cmp dword ptr gs:[0], 0B0BACAFEh
    jne InterruptReturnFromInterruptCodeNoRestore
    swapgs

InterruptReturnFromInterruptCodeNoRestore:
    add rsp, 010h
    iretq

KxReturnFromInterrupt ENDP

; dont even look at this

KxRollingIsr PROC

    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    add qword ptr [rsp + 08h], 010000h
    push 0ffffh
    jmp KxPreprocessInterrupt

KxRollingIsr ENDP

END