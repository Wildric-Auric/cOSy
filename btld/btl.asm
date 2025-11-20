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
; 0x8600-0x7FFF--> space used by stage 2 (see next lines)
; 0x8600-0x8A00--> data about VBE
; 0x8A00-0x8A02--> count of memory map
; 0x8A02- ...  --> memory map output

; changed->
; 0x8600-0x7FFF--> kernel  
;
; when A20 is enabled:
; 0xFFFF:0x10=0x100000 ---> Kernel
; 0x00F00000--->stack

rmd_stack_offset    equ 0x7BFF
pmd_stack_offset    equ 0xF00000
stg2_offset         equ 0x7E00
stg2_data_offset    equ 0x8600
kernel_offset       equ 0x100000
vesa_offset         equ stg2_data_offset
vesa_mode_offset    equ vesa_offset + 512


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
wrt_char:
    pusha
    mov ah, 0x0E
    mov al, bl
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
    mov bl, ah
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
    mov  bx, 510 + 0x7E00
    call check_a20
    cmp ax, 0x1 
    je enable_a20_end
    ; fast gate
    in al, 0x92
    or al, 2
    out 0x92, al

    mov  bx, 510 + 0x7E00
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
    mov al, 72 
    mov cl, 5
    call ldsk
    ; get free memory map
    call get_mem_map
    ; Do it right before PM to continue using VGA before
    ; Enable VESA
    call enable_vesa 
    ; switch to pm
    call switch_to_pm 
    ret

; https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
get_mem_map:
    pusha
    mov ax, 0
    push ax
    ; ES:DI -> 0x8A00
    mov ax, 0x8A0 
    mov es, ax
    mov di, 2
    ; clear ebx
    xor ebx, ebx
get_mem_map_lp:
    ; set edx
    mov edx, 0x534D4150 
    ; set eax
    mov eax, 0x0000E820
    ; set ecx
    mov ecx, 0x18
    int 0x15
    jc get_mem_map_err
    cmp ebx, 0x0 
    je get_mem_map_end
    ; next ieration setup
    xor ah, ah
    mov al, 0x18 
    add di, ax
    pop  ax 
    add ax, 1
    push ax
    jmp get_mem_map_lp
get_mem_map_end:
    ; print info
    pop ax
    mov bx, mem_count_str
    wrts mem_count_str 
    wrtn ax
    call wrt_endl
    ; store memory count
    push es 
    mov bx, 0x8A0
    mov es, bx 
    xor bx, bx
    mov [es:bx], ax
    pop es
    ; return
    popa
    ret

get_mem_map_err:
    wrts err_str
    popa
    ret
    
get_vesa_info:
    pusha
    mov ax, 0x4F00
    mov di, vesa_offset
    mov bx, 0x0 
    mov es, bx
    mov dword[es:di], 'VBE2'
    int 0x10
    cmp ax, 0x004F
    jne get_vesa_info_err
    wrtn ax
    popa
    ret
get_vesa_info_err:
    wrts vesa_err_str  
    jmp $
    popa
    ret
; input cx
; output ax 0x4F on success 
get_vesa_mode_info:
    pusha
    mov ax, 0x4F01
    mov di, vesa_mode_offset
    mov bx, 0x0
    mov es, bx
    ; mov cx, input
    int 0x10
    mov [vesa_mode_ret_val], ax
    popa
    ret
vesa_mode_ret_val:
    dw 0
enable_vesa:
        pusha
        call get_vesa_info
        push ds
        mov ax, 0x0 
        mov ds, ax
        mov bx, [vesa_offset + 14]
        mov es, [vesa_offset + 16]
        pop ds
    enable_vesa_lp:
        mov cx, [es:bx] 
        cmp cx, 0xFFFF 
        je enable_vesa_end
        ; check mode information
        call get_vesa_mode_info
        cmp [vesa_mode_ret_val], 0x4F
        je .next
        call vesa_mode_not_supported
        jmp enable_vesa_lp 
        .next:
        push bx
        pop bx
        call enable_vesa_choose
        add bx, 2
        jmp enable_vesa_lp
    enable_vesa_end:
        cmp [vesa_sel_width], 0x0
        je  get_vesa_info_err
        ; store choosen mode info in memory
        mov es, [vesa_sel_vid_mode_seg]
        mov bx, [vesa_sel_vid_mode_offset]
        mov cx, [es:bx]
        call get_vesa_mode_info 
        cmp [vesa_mode_ret_val], 0x4F
        jne vesa_mode_not_supported
        popa
        ; print choosen mode info
        xor dx, dx
        mov bx, [vesa_sel_width]
        mov cx, [vesa_sel_height]
        mov dl, byte[vesa_sel_bpp]
        call wrt_endl
        wrtn bx 
        wrtn cx 
        wrtn dx
        ;------------------------
        mov ax, 0x4F02
        mov es, [vesa_sel_vid_mode_seg]
        mov bx, [vesa_sel_vid_mode_offset]
        mov cx, [es:bx]
        call get_vesa_mode_info 
        ; ret ; todo
        mov bx, cx
        and bx, 0x3FF
        or  bx,  0x4000 
        int 0x10
        cmp ax, 0x004F
        jne get_vesa_info_err 
        ret
enable_vesa_choose:
    pusha
    push ds
    ; check linear bit 
    mov ax, 0x0
    mov ds, ax
    mov al, byte[vesa_mode_offset]
    and al, 0x80
    cmp al, 0x80
    jne .end_vesa_choose
    mov ax, word[vesa_mode_offset + 0x28]
    or  ax, word[vesa_mode_offset + 0x2A]
    cmp ax, 0
    je  .end_vesa_choose
    xor ax, ax
    xor dx, dx
    xor cx, cx
    xor dx, dx

    mov bx, word[vesa_mode_offset + 0x12]
    mov cx, word[vesa_mode_offset + 0x14]
    mov dl, byte[vesa_mode_offset + 0x19]
    ;mov al, byte[vesa_sel_bpp]
    mov al, 0x20
    cmp dl, al
    jne .end_vesa_choose
    ; jl .end_vesa_choose
    mov ax, 0x7E0
    mov ds, ax 
    mov [vesa_sel_width]  , bx
    mov [vesa_sel_height] , cx
    mov byte[vesa_sel_bpp], dl
    ; should save
    pop ds
    popa 
    push ax
    mov ax, [es:bx]
    mov [vesa_sel_vid_mode_offset], bx
    mov [vesa_sel_vid_mode_seg],    es
    pop ax
    ret
    .end_vesa_choose:
    pop ds
    popa
    ret
vesa_mode_not_supported:
    pusha
    ;wrts vesa_mode_support_str
    ;wrtn cx
    popa
    ret


vesa_sel_vid_mode_seg:
    dw 0
vesa_sel_vid_mode_offset:
    dw 0
vesa_sel_width:
    dw 0
vesa_sel_height:
    dw 0
vesa_sel_bpp:
    db 0

[bits 16]
switch_to_pm:
    cli 
    lgdt [gdt_desc]
    mov eax, cr0
    or  eax, 0x1

    mov cr0, eax
    jmp code_seg:(init_pm + 0x7E00)
    ret

mem_count_str:
    db "Memory count: ",0
vesa_err_str:
    db "VESA Bios not supported",0
vesa_mode_support_str:
    db "VESA mode not supposed: ",0
OK_str:
    db "OK",0

; -------------------------------32 Bit PM-------------------------------
[bits 32]
init_pm:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; mov ebp, 0x7C00 ; stack must be divisible by 16
    mov ebp, 0xF00000
    mov esp, ebp
    mov ebx, pm_str + 0x7E00
    ; call wrt_str32
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


;vbe_info:
;    .signature db "VBE2"
;    .dat       resb 512 - 4
;struct vbe_info_structure {
; 4 char[4] signature = "VESA";	// must be "VESA" to indicate valid VBE support
; 6 uint16_t version;			// VBE version; high byte is major version, low byte is minor version
;10 uint32_t oem;			    // segment:offset pointer to OEM
;14	uint32_t capabilities;		// bitfield that describes card capabilities
;18	uint32_t video_modes;		// segment:offset pointer to list of supported video modes
;20	uint16_t video_memory;		// amount of video memory in 64KB blocks
;	uint16_t software_rev;		// software revision
;	uint32_t vendor;			// segment:offset to card vendor string
;	uint32_t product_name;		// segment:offset to card model name
;	uint32_t product_rev;		// segment:offset pointer to product revision
;	char reserved[222];		// reserved for future expansion
;	char oem_data[256];		// OEM BIOSes store their strings in this area
;} __attribute__ ((packed));

;struct vbe_mode_info_structure {
;	0x00  uint16_t attributes;
;	0x02  uint8_t  window_a;
;	0x03  uint8_t  window_b;
;	0x04  uint16_t granularity;
;	0x06  uint16_t window_size;
;	0x08  uint16_t segment_a;
;	0x0A  uint16_t segment_b;
;	0x0C  uint32_t win_func_ptr;
;	0x10  uint16_t pitch;
;	0x12  uint16_t width;
;	0x14  uint16_t height;
;	0x16  uint8_t  w_char;
;	0x17  uint8_t  y_char;
;	0x18  uint8_t  planes;
;	0x19  uint8_t  bpp;
;	0x1A  uint8_t  banks;
;	0x1B  uint8_t  memory_model;
;	0x1C  uint8_t  bank_size;
;	0x1D  uint8_t  image_pages;
;	0x1E  uint8_t  reserved0;
;
;	0x1F  uint8_t  red_mask;
;	0x20  uint8_t  red_position;
;	0x21  uint8_t  green_mask;
;	0x22  uint8_t  green_position;
;	0x23  uint8_t  blue_mask;
;	0x24  uint8_t  blue_position;
;	0x25  uint8_t  reserved_mask;
;	0x26  uint8_t  reserved_position;
;	0x27  uint8_t  direct_color_attributes;
;
;	0x28  uint32_t framebuffer;
;	0x2C  uint32_t off_screen_mem_off;
;	0x30  uint16_t off_screen_mem_size;
;	0x32  uint8_t  reserved1[206];
;} __attribute__ ((packed));
