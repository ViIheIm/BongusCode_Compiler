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
sub rsp, 24



; Body

; _t0 = 7
mov QWORD PTR 8[rsp], 7
mov rax, QWORD PTR 8[rsp]

; _t1 = 3
mov QWORD PTR 16[rsp], 3
mov rax, QWORD PTR 16[rsp]

; _t0 *= _t1
mov rax, QWORD PTR 8[rsp]
imul rax, QWORD PTR 16[rsp]
mov QWORD PTR 8[rsp], rax

; (local var at stackLoc 0) = Result of expr(rax)
mov QWORD PTR 0[rsp], rax

; Return expression:
; _t0 = (local var at stackLoc 0)
mov rax, QWORD PTR 0[rsp]
mov QWORD PTR 8[rsp], rax

; _t1 = 1
mov QWORD PTR 16[rsp], 1
mov rax, QWORD PTR 16[rsp]

; _t0 -= _t1
mov rax, QWORD PTR 8[rsp]
sub rax, QWORD PTR 16[rsp]
mov QWORD PTR 8[rsp], rax

; _t2 = 2
mov QWORD PTR 24[rsp], 2
mov rax, QWORD PTR 24[rsp]

; _t0 -= _t2
mov rax, QWORD PTR 8[rsp]
sub rax, QWORD PTR 24[rsp]
mov QWORD PTR 8[rsp], rax
; Dealloc section for local variables.
add rsp, 8
; Dealloc section for temporary variables.
add rsp, 24
mov rsp, rbp
pop rbp
ret
; TODO: Hard coded name, bad!
main ENDP
END