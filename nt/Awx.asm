.code

CpuxGetCurrentApicId PROC
	push rbx
	mov eax, 01h	; what if my cpu doesnt support leaf 01h??????
					; its ok the og nt gang didnt think this was possible i am just like them
	cpuid
	shr ebx, 24
	mov eax, ebx
	pop rbx
	ret

CpuxGetCurrentApicId ENDP

AwxReadIcr PROC
	mov eax, [rcx+310h]
	mov [rdx], eax

	mov eax, [rcx+300h]
	mov [r8], eax
	ret

AwxReadIcr ENDP

; the sdm specifies a mov instruction is needed when writing to LAPIC+0300h
; the compiler sometimes output smth else. id assume it would work but i havent actually tested
; *(unsigned int)(icr) should probs work tho
; im lazy eepy baby

AwxWriteIcr PROC
	mov [rcx+310h], r8d
	mfence	; this is probably useless idk how x86 works
	mov [rcx+300h], edx
	ret

AwxWriteIcr ENDP

END