OPTION DOTNAME   ; Allows the use of dot notation(MASM64 requires this for 64 - bit assembly)
.data


.code



; Prologue
; TODO: Hard coded name, bad!
main PROC
push rbp
mov rbp, rsp
; Alloc section for local variables.
sub rsp, 8
; Alloc section for temporary variables.
sub rsp, 16



; Body

; _t0 = 7
mov QWORD PTR 8[rsp], 7
mov rax, QWORD PTR 8[rsp]

; _t1 = 2
mov QWORD PTR 16[rsp], 2
mov rax, QWORD PTR 16[rsp]

; _t0 *= _t1
mov rax, QWORD PTR 8[rsp]
imul rax, QWORD PTR 16[rsp]
mov QWORD PTR 8[rsp], rax

; (local var at stackLoc 0) = Result of expr(rax)
mov QWORD PTR 0[rsp], rax



; Epilogue
; Dealloc section for local variables.
add rsp, 8
; Dealloc section for temporary variables.
add rsp, 16
mov rsp, rbp
pop rbp
ret
; TODO: Hard coded name, bad!
main ENDP
END