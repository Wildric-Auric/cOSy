#include "inter.h"
#include "vga.h"


extern uchar rd_port_byte(ushort port);
extern void wrt_port_byte(ushort port, uchar data);


idt_gate idt[256];
idt_reg  idt_info;
void (*irq_hdl_cbks[256])(isr_hdl_args);

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

extern void isr0();    extern void isr1();
extern void isr2();    extern void isr3();
extern void isr4();    extern void isr5();
extern void isr6();    extern void isr7();
extern void isr8();    extern void isr9(); 
extern void isr10();   extern void isr12();
extern void isr11();   extern void isr12();
extern void isr13();   extern void isr14();
extern void isr15();   extern void isr16();
extern void isr17();   extern void isr18();
extern void isr19();   extern void isr20(); 
extern void isr21();   extern void isr22();

extern void isr23();   extern void isr24();
extern void isr25();   extern void isr26();
extern void isr27();   extern void isr28();
extern void isr29();   extern void isr30();
extern void isr31();   

extern void irq0();    extern void irq1();
extern void irq2();    extern void irq3();
extern void irq4();    extern void irq5();
extern void irq6();    extern void irq7();
extern void irq8();    extern void irq9(); 
extern void irq10();   extern void irq12();
extern void irq11();   extern void irq12();
extern void irq13();   extern void irq14();
extern void irq15();   

#define SET_GATE(i) set_idt_gate(i, (ui32)&isr##i);
#define SET_GATE2(i) set_idt_gate(i+32, (ui32)&irq##i);
void set_idt() {
    SET_GATE(0); SET_GATE(1);   SET_GATE(2);
    SET_GATE(3); SET_GATE(4);   SET_GATE(5);
    SET_GATE(6); SET_GATE(7);   SET_GATE(8);
    SET_GATE(9); SET_GATE(10);  SET_GATE(11);
    SET_GATE(12); SET_GATE(13); SET_GATE(14);
    SET_GATE(15); SET_GATE(16); SET_GATE(17);
    SET_GATE(18); SET_GATE(19); SET_GATE(20);
    SET_GATE(21); SET_GATE(22); SET_GATE(23);
    SET_GATE(24); SET_GATE(25); SET_GATE(26);
    SET_GATE(27); SET_GATE(28); SET_GATE(29);
    SET_GATE(30); SET_GATE(31); 

    SET_GATE2(0); SET_GATE2(1);   SET_GATE2(2);
    SET_GATE2(3); SET_GATE2(4);   SET_GATE2(5);
    SET_GATE2(6); SET_GATE2(7);   SET_GATE2(8);
    SET_GATE2(9); SET_GATE2(10);  SET_GATE2(11);
    SET_GATE2(12); SET_GATE2(13); SET_GATE2(14);
    SET_GATE2(15); 
    
    //Remap the PIC
    wrt_port_byte(0x20, 0x11);
    wrt_port_byte(0xA0, 0x11);
    wrt_port_byte(0x21, 0x20);
    wrt_port_byte(0xA1, 0x28);
    wrt_port_byte(0x21, 0x04);
    wrt_port_byte(0xA1, 0x02);
    wrt_port_byte(0x21, 0x01);
    wrt_port_byte(0xA1, 0x01);
    wrt_port_byte(0x21, 0x0);
    wrt_port_byte(0xA1, 0x0);     

    load_idt();
}

void isr_hdl(isr_hdl_args arg) {
    vga_print("Interrupt received: ");
    vga_print16(arg.num);
    vga_go_next_line();
}

void bind_int_hdl(ui8 i, void(*f)(isr_hdl_args)) {
    irq_hdl_cbks[i] = f;
}

void irq_hdl(isr_hdl_args arg) {
    if (arg.num > 39)
    wrt_port_byte(0xA0, 0x20);
    wrt_port_byte(0x20, 0x20);
    if (!irq_hdl_cbks[arg.num]) return;
    irq_hdl_cbks[arg.num](arg);
}
