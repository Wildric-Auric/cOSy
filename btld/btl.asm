;mov ax, 0x7C0
;mov ds, ax
[org 0x7c00]
KERNEL_OFFSET equ 0x1000
mov [BOOT_DRIVE], dl

mov bp, 0x8000
mov sp, bp   ; sets the stack on the end of 2nd sector

;main

mov  bx, txt 
call wrtstr
call wrtendl


;mov ax, 0x0
;mov es, ax
;mov bx, bp 
;mov dh, 2
;call ldsk

call ldkrnl 
call swtch_pm

jmp eop

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
    
    mov cx, 0x1000

    wrtnum16lp:
        mov ax, dx
        xor dx, dx
        div cx
        add ax, hexlkup 
        mov bx, ax 
        mov ah, 0xE
        mov al, [bx]
        int 0x10
        shr cx, 0x04
        cmp cx, 0x0
        jne wrtnum16lp 
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
    mov cl, 0x02 ; 0x1 would be the bootloader sector 
    mov ch, 0x0
    mov dh, 0x0
    mov dl, [BOOT_DRIVE]
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

;ldkrnl: loads kernel from disk 
ldkrnl:
    mov ax, 0x0
    mov es, ax
    mov bx, KERNEL_OFFSET 
    mov dh, 2
    call ldsk
    ret

    
;------------------------32 bit PM------------------------
; https://wiki.osdev.org/GDT_Tutorial#What_to_Put_In_a_GDT
; https://wiki.osdev.org/Global_Descriptor_Table
; total base is 16, total limit 20 bit
gdt_beg:
    ; Entry 0, has only 0s
    dd 0x0 ;4 bytes
    dd 0x0 
    ; Code seg desc
gdt_code:
    dw 0xFFFF     ;length 0-15  16
    dw 0x0        ;base   16-31 16
    db 0x0        ;base   32-39 8
    db 10011010b  ;access byte 40-47  8
    db 11001111b  ;flags+limit 48-55 4+4
    db 0x0        ;base    55-63 8

    ; Data Seg
gdt_dat:
    dw 0xFFFF     
    dw 0x0        
    db 0x0        
    db 10010010b  
    db 11001111b  
    db 0x0        
gdt_end:

gdt_desc:
    dw gdt_end - gdt_beg - 1    ;gdt size minus -1 
    dd gdt_beg                  ;address 8 bytes

CODE_SEG equ gdt_code - gdt_beg
DATA_SEG equ gdt_dat  - gdt_beg

[bits 16]
swtch_pm:
    cli ;disable ints 
    lgdt [gdt_desc] ;load gdt
    ; set cr0 first bit to 1
    mov eax, cr0
    or  eax,  0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm ;far jump different seg

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    call beg_32pm 

;------------------------32 bit code------------------------
[bits 32]
wrtstr32:
    pusha
    mov ah,  0x0F    ;color
    mov edx, 0xB8000
    wrtstr32lp:
        mov al, [ebx] ;char 
        cmp al, 0
        je  wrtstr32end
        mov [edx], ax
        add edx, 2
        add ebx, 1
        jmp wrtstr32lp
    wrtstr32end:
        popa
        ret

beg_32pm:
mov ebx, beg32pm
call wrtstr32
mov ah,  0x0F    ;color
mov edx, 0xB8000
mov eax, KERNEL_OFFSET
call KERNEL_OFFSET 
mov ebx, end32pm 
call wrtstr32
jmp $
;----------------------------------------------------------

%define endl 0xA,0xD,0

BOOT_DRIVE db 0
beg32pm: 
    db "Entered 32-bit Protected mode",0
end32pm: 
    db "back from kernel",0
hexlkup:
    db "0123456789ABCDEF",0
txt:
    db "Hello BootZector",0 
ldsk_err_str:
    db "Read Disk Error",0
ldsk_ok:
    db "Read Disk Ok",0

eop:
    jmp $
times 510-($-$$) db 0
dw    0xAA55
