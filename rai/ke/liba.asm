.code

; WHY THE FUCK DOES MSVC NOT HAVE INLINE ASM ANYMORE
; WHO AT MICROSOFT DECIDED YEAH JUST USE MASM 
; MASM IS DOGSHIT
; WHAT IS PROC AND ENDP
; "who needs to swapgs for a driver wah wah"

KiSwapGs PROC

	swapgs
	ret

KiSwapGs ENDP

END