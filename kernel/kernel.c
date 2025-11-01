#include "inter.h"
#include "util.h"
#include "kb.h"
#include "vga.h"


#define VIDEO_MEMORY  0xb8000

typedef struct  {
    int cur; 
    int width;
} TTY_Data;

uchar rd_port_byte(ushort port) {
    uchar res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}
void wrt_port_byte(ushort port, uchar data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

volatile ui32 tick;
void timer_isr_cbk(isr_hdl_args arg) {
    ++tick;
    vga_clear();
    vga_print32(tick);
}
void drv_init_timer(ui32 frq) {
    bind_int_hdl(32, timer_isr_cbk);
    ui32 d = 1193180 / frq;
    wrt_port_byte(0x43, 0x36);
    wrt_port_byte(0x40, d & 0xFF);
    wrt_port_byte(0x40, (d>>8) & 0xFF);
}

void basic_text_editor() {
    while (1) {
    while (kb_get_stack_ptr()) {
        ui16    sc   = kb_pop_scode();
        key_inf ki;
        kb_cnv_scode(sc, &ki);
        boolean prs = ki.event & key_event_enm_pressed;
        boolean isan= (ki.key >= 'a' && ki.key <= 'z') || (ki.key >= '0' && ki.key <= '9');
        if (prs && isan) {
            ki.key = cnv_ascii_qwaz(ki.key);
            vga_print_char(ki.key);
        }
        if (ki.key == Enter_Key && prs)
            vga_go_next_line();
        if (ki.key == Space_Key && prs) 
            vga_print_char(' ');
        if (ki.key == Back_Key  && prs) {
            vga_ptr_vd->cur -= 2;
            vga_print_char(' ');
            vga_ptr_vd->cur -= 2;
        }
    }
    }
}

const char* a = "Hey there";
void main() {
    drv_init_vga();
    vga_clear();
    set_idt();
    asm __volatile__("sti");
    drv_init_kb(); 
          
    vga_go_next_line();
    for (int i = 0; i < vga_ptr_vd->width; ++i) {
        vga_print("-"); 
    }
    vga_go_next_line();
    //vga_print(a); vga_print(", From C!");
    vga_go_next_line();
    vga_update_gcur();  

    basic_text_editor();
}
