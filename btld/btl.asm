[bits 16]
%macro wrtn 1
    mov bx, %1
    call wrt_num16
    call wrt_endl
%endmacro
%macro wrts 1
    mov bx, %1
    call wrt_str
    call wrt_endl
%endmacro
call init_boot_sector
jmp $
; https://wiki.osdev.org/Memory_Map_(x86)
; 0x00000500	0x00007BFF	29.75 KiB	Conventional mem(we put stack here)
; 0x00007C00	0x00007DFF	512 bytes	BootSector  (bootloader)
; 0x00007E00	0x0007FFFF	480.5 KiB	Conventional mem (we put second stage here)

; 0x500-0x7BFF --> stack
; 0x7C00-0x7DFF--> bootsector
; 0x7E00-0x8600--> stage 2 (2048 bytes)
; 0x8600-0x7FFF--> kernel  
;
; when A20 is enabled:
; 0xFFFF:0x10=0x100000 ---> Kernel
; 0x00F00000--->stack
init_boot_sector:
    mov [boot_drive], dl
    mov ax, 0x7C0
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov bp, 0x7C00
    mov sp, bp
    wrts boot_sector_str
    call load_stage2
    wrts err_str
    ret

;   al: number of sectors to load
;   cl: base sector number. 1 for bootsector
;   bx: base dst address to load to
;   es: dst address segment
load_stage2:
    xor bx, bx
    mov ax, 0x7E0
    mov es, ax 
    mov al, 4
    mov cl, 1
    call ldsk
    mov dl, [boot_drive]
    jmp 0x7E0:stage2_start
    ret
; ------------------Common func------------------
wrt_endl:
    pusha
    mov ah, 0x0E
    mov al, 0xD
    int 0x10
    mov al, 0xA
    int 0x10
    popa
    ret
wrt_str:
    pusha
   wrt_str_lp:
        mov ah, 0x0E
        mov al, [bx]
        add bx, 1
        cmp al, 0
        je wrt_str_end
        int 0x10 
        jmp wrt_str_lp
   wrt_str_end:
    popa
    ret
wrt_num16:
    pusha
    mov ah, 0xE
    mov al, "0"
    int 0x10
    mov ah, 0xE
    mov al, "x"
    int 0x10
    mov dx, bx
    mov cx, 0x1000
    wrt_num16lp:
        mov ax, dx
        xor dx, dx
        div cx
        add ax, hex_lkup 
        mov bx, ax 
        mov ah, 0xE
        mov al, [bx]
        int 0x10
        shr cx, 0x04
        cmp cx, 0x0
        jne wrt_num16lp 
    popa
    ret
;ldsk: load memory from disk
;      args:
;           al: number of sectors to load
;           cl: base sector number. 1 for bootsector
;           bx: base dst address to load to
;           es: dst address segment
;   Doc for 
;   INT 0x13 ah=02: 
;	AL = number of sectors to read	(1-128 dec.)
;	CH = track/cylinder number  (0-1023 dec., see below)
;	CL = sector number  (1-17 dec.)
;	DH = head number  (0-15 dec.)
;	DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
;	ES:BX = pointer to buffer
;   Note: DL is set by the bios, one should not overwrite it

ldsk:
    pusha
    push ax
    mov ah, 0x02
    mov ch, 0x0
    mov dh, 0x0
    mov dl, [boot_drive]
    int 0x13
    jc  ldsk_err
    pop bx
    cmp al, bl 
    jne ldsk_err
    wrts ldsk_ok
    popa
    ret
ldsk_err:
    mov bx, ldsk_err_str
    call wrt_str
    xor bx, bx 
    mov bh, ah
    call wrt_num16
    popa
    ret
; check_a20: checks if line a20 is enabled.
;       args:
;           bx: address to check and to mutate
;       ret:
;           ax: 0 if line a20 is disabled 1 if enabled
; http://wiki.osdev.org/A20_Line
check_a20:
    pushf
    push ds
    push es
    push di
    push si
    cli
    xor ax, ax
    mov es, ax
    not ax
    mov ds, ax  
    ; ds = 0xFFFF
    ; es = 0x0000
    mov di, bx  ; address to check
    mov si, di
    add si, 0x10
    mov al, byte[es:di] ; 0x0     + 0xXX
    push ax
    mov al, byte[ds:si] ; 0xFFFF0 + 0xXX + 0x10
    ; diff is 1 mb
    push ax
    mov byte[es:di], 0xDD
    mov byte[ds:si], 0xFF
    cmp byte[es:di], 0xFF
    ; restore values
    pop ax
    mov [ds:si], al
    pop ax
    mov [es:di], al
    ; xor ax, ax -->does not work it affects the ZF flag
    mov ax, 0
    je check_a20_end 
    mov ax, 1
    check_a20_end:
        sti
        pop si
        pop di
        pop es
        pop ds
        popf
        ret

enable_a20:
    pusha
    call check_a20
    cmp ax, 0x1 
    je enable_a20_end
    ; fast gate
    in al, 0x92
    or al, 2
    out 0x92, al

    call check_a20
    cmp ax, 0x1 
    jne enable_a20_err 
    ; TODO::add other ways to enable a20 line
enable_a20_end:
    popa
    ret
enable_a20_err:
    wrts a20_err_str
    popa
    jmp $

; ------------------data------------------
boot_drive: 
    db 0
hex_lkup:
    db "0123456789ABCDEF",0
boot_sector_str:
    db "Boot Sector executing",0
stage2_str:
    db "Stage 2 executing", 0
err_str:
    db "Undefined Error", 0
ldsk_ok:
    db "Read disk success",0
ldsk_err_str:
    db "Error loading disk: ",0
a20_err_str:
    db "Could not continue, A20 enabling impossible.",0
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
    dw 0x0        ;0x7E00
    db 0x0        
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
    dd gdt_beg + 0x7E00         ;address 8 bytes
code_seg equ gdt_code - gdt_beg
data_seg equ gdt_dat  - gdt_beg


; ------------------------------------
times 510 - ($-$$) db 0
dw 0xAA55
; ----------------stage2-------------------
stage2_start:
    call init_stage2
    jmp $

init_stage2:
    ; set regs 
    mov [boot_drive], dl 
    mov ax, 0x7E0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    wrts stage2_str
    ; enable A20 line
    call enable_a20
    ; load kernel
    xor bx, bx
    ; loads the kernel at 1 MB - 0xFFFF:0x10 = 0x100000
    ; mov ax, 0x860 ; load kernel to 0x8600
    mov ax, 0xFFFF 
    mov es, ax 
    mov bx, 0x10
    mov al, 16 
    mov cl, 5
    call ldsk
    ; switch to pm
    call switch_to_pm 
    ret

[bits 16]
switch_to_pm:
    cli 
    lgdt [gdt_desc]
    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax
    jmp code_seg:(init_pm + 0x7E00)
    ret

[bits 32]
init_pm:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x7C00 ;stack must be divisible by 16
    ; mov ebp, 0x00F00000
    mov esp, ebp
    mov ebx, pm_str + 0x7E00
    call wrt_str32
    call 0xF8200
    ret
wrt_str32:
    pusha
    mov ah,  0x0F    ;color
    mov edx, 0xB8000
    wrt_str32_lp:
        mov al, [ebx] ;char 
        cmp al, 0
        je  wrt_str32_end
        mov [edx], ax
        add edx, 2
        add ebx, 1
        jmp wrt_str32_lp
    wrt_str32_end:
        popa
        ret
pm_str:
    db "32-bit pm executing",0

times 2046 - ($-$$) db 0
dw 0x6969
