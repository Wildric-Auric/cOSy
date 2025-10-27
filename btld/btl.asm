;int 0x10 writes tty after setting a byte on al

[org 0x7c00]

mov ah, 0x0e ;tty
mov bp, 0x8000
mov sp, bp   ; sets the stack on the end of 2nd sector


;main


mov  bx, txt 
call wrtstr
call wrtendl


mov ax, 0x0
mov es, ax
mov bx, 0x8000
mov dh, 2
call ldsk


mov bx, [0x8000]

call wrtendl
call wrtnum16
call wrtendl
call wrtendl

jmp $

wrtendl:
    pusha
    mov ah, 0xE
    mov al, 0xA
    int 0x10
    mov al, 0xD
    int 0x10
    popa
    ret

;functions
;wrtstr: write line.
;        args: 
;             bx: address of the string
wrtstr: 
    pusha
    mov bx, bx 
    wrtlp:
        mov ah, 0xE
        mov al, [bx]
        add bx, 1
        cmp al, 0
        je endwrtstr
        int 0x10
        jmp wrtlp
    endwrtstr:
        call wrtendl
        popa
        ret

;wrtnum: write number
;        args:
;               bx: number
; Unsigned divide DX:AX by r/m16, 
; with result stored in AX := Quotient, DX := Remainder.
; if div quotient overflows, real mode processors halt
wrtnum16:
    pusha

    mov ah, 0xE
    mov al, "0"
    int 0x10
    mov ah, 0xE
    mov al, "x"
    int 0x10

    mov dx, bx

    mov ax, dx 
    mov cx, 0x1000
    xor dx, dx
    div cx
    add ax, hexlkup 
    mov bx, ax 
    mov ah, 0xE
    mov al, [bx]
    int 0x10

    mov ax, dx 
    mov cx, 0x0100
    xor dx, dx
    div cx
    add ax, hexlkup 
    mov bx, ax 
    mov ah, 0xE
    mov al, [bx]
    int 0x10

    mov ax, dx 
    mov cx, 0x10
    xor dx, dx
    div cx
    add ax, hexlkup 
    mov bx, ax 
    mov ah, 0xE
    mov al, [bx]
    int 0x10

    mov ax, dx 
    mov cx, 0x1
    xor dx, dx
    div cx
    add ax, hexlkup 
    mov bx, ax 
    mov ah, 0xE
    mov al, [bx]
    int 0x10 

    popa
    ret

;ldsk: load memory from disk
;      args:
;           dh: number of sectors to load
;           bx: base dst RAM adress to load to
;   int 0x13 AH = 02
;	AL = number of sectors to read	(1-128 dec.)
;	CH = track/cylinder number  (0-1023 dec., see below)
;	CL = sector number  (1-17 dec.)
;	DH = head number  (0-15 dec.)
;	DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
;	ES:BX = pointer to buffer
;   Note: DL is set by the bios, one should not overwrite it

ldsk:
    pusha
    push dx 
    mov ah, 0x02
    mov al, dh 
    mov cl, 0x01
    mov ch, 0x0
    mov dh, 0x0
    int 0x13
    jc  ldsk_err
    pop dx 
    cmp al, dh
    jne ldsk_err
    mov bx, ldsk_ok
    call wrtstr 
    popa
    ret
ldsk_err:
    mov bx, ldsk_err_str
    call wrtstr
    xor bx, bx
    mov bh, ah
    call wrtnum16
    popa
    ret
    




%define endl 0xA,0xD,0


hexlkup:
    db "0123456789ABCDEF",0
txt:
    db "Hello BootSector",0 
ldsk_err_str:
    db "Read Disk Error",0
ldsk_ok:
    db "Read Disk Ok",0

times 510-($-$$) db 0
dw    0xAA55
