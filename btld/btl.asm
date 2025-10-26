[org 0x7c00]


mov ah, 0x0e ;tty

;mov al, 'h'
;int 0x10
;mov al, 'E'
;int 0x10
;mov al, 'L'
;int 0x10
;int 0x10

mov dx, [txtsize] 
mov cx, 0 

wrtl: 
mov bx, txt
add bx, cx 
mov al, [bx]
int 0x10 
add cx, 1 
cmp cx, dx 
jne wrtl

jmp $

txt:
    db "Hello World"
txtsize:
    db 11
times 510-($-$$) db 0
dw    0xAA55
