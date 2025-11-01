#ifndef  INTER_H
#define  INTER_H

#include "globals.h"

typedef struct  {
   ui32 ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
   ui32 num;
} isr_hdl_args;

// https://wiki.osdev.org/Interrupt_Descriptor_Table
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


void bind_int_hdl(ui8 i, void(*f)(isr_hdl_args));
void set_idt();

#endif
