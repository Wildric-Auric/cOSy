[org 0x7c00]


mov ah, 0x0e ;tty

;mov al, 'h'
;int 0x10

;main

mov  bx, txt 
call wrtl

jmp $

;functions
;wrtstr: write line.
;        args: 
;             bx: contains address of the string
wrtstr: 
mov bx, bx 
wrtlp:
mov al, [bx]
add bx, 1
cmp al, 0
je endwrtstr
int 0x10
jmp wrtstr
endwrtstr:
ret





txt:
    db "Hello BootSector",0
txtsize:
    db 11
times 510-($-$$) db 0
dw    0xAA55
