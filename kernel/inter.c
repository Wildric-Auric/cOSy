#include "globals.h"


extern int print_str(const char* str);
extern void print_hex16(int n);

//https://wiki.osdev.org/Interrupt_Descriptor_Table
typedef struct {
    ui16 offset_low;
    ui16 sel;
    ui8  _reserved;
    ui8  flags; //gate type + 0 + DPL + 1 (P present for valid descriptor)
    ui16 offset_high;
} __attribute__((packed)) idt_gate;
typedef struct {
    ui16 size;   //SIZE of IDT in bytes - 1
    ui32 offset; //linear address of idt (paging applies)
} __attribute__((packed)) idt_reg; //idt register (IDTR)

idt_gate idt[256];
idt_reg  idt_info;


void set_idt_gate(int n, ui32 hdl) {
    idt[n].offset_low = hdl & 0xFFFF;
    idt[n].sel        = 0b1000; //2 bits -> 0 highest privilege, 1 bit -> 0 uses gdt, index 1 uses first valid entry which is code
    idt[n]._reserved  = 0;
    idt[n].flags      = 0x8E; //interrupt table 0xE(not trap)  + 0 + 0 level priority + 1 Present
    idt[n].offset_high= (hdl >> 16) & 0xFFFF;
}

void load_idt() {
    idt_info.size   = sizeof(idt_gate) * 256 - 1;
    idt_info.offset = (ui32)&idt;
    __asm__ __volatile__("lidtl (%0)" : : "r" (&idt_info));
}




extern void isr0();extern void isr1();
extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5();
extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); 
extern void isr10();
#define SET_GATE(i) set_idt_gate(i, (ui32)&isr##i);
void set_idt() {
    SET_GATE(0); SET_GATE(1); SET_GATE(2);
    SET_GATE(3); SET_GATE(4); SET_GATE(5);
    SET_GATE(6); SET_GATE(7); SET_GATE(8);
    SET_GATE(9); SET_GATE(10);
    load_idt();
}

typedef struct  {
   ui32 ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
   ui32 num;
} isr_hdl_args;

void isr_hdl(isr_hdl_args arg) {
    print_str("Interrupt received: ");
    print_hex16(arg.num);
}
