#include "inter.h"
#include "util.h"
#include "kb.h"
#include "video.h"


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





//--------------Sample programs-----------------
void basic_text_editor() {
    while (1) {
    while (kb_get_stack_ptr()) {
        ui16    sc   = kb_pop_scode();
        key_inf ki;
        kb_cnv_scode(sc, &ki);
        boolean prs  = ki.event & key_event_enm_pressed;
        boolean isan = is_alpha_num(ki.key);
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
//-------------------------------
#define particle_num 48 
#define ptimer 9000
i3 sands[particle_num];

void sand_gui() {
    for (int i = 0; i < particle_num; ++i) {
        sands[i].x = -1;
        sands[i].y = -1;
    }

    while (1) {
        int x,y;
        while (kb_get_stack_ptr()) {
            ui16    sc   = kb_pop_scode();
            key_inf ki;
            kb_cnv_scode(sc, &ki);
            boolean prs  = ki.event & key_event_enm_pressed;
            boolean rls  = ki.event & key_event_enm_released;
            boolean isan = is_alpha_num(ki.key);
            vga_get_cur_pos2(&x,&y);
            if (prs && isan) {
                ki.key = cnv_ascii_qwaz(ki.key);
            }
            if (prs && ki.key == 'l') {
                vga_set_col(0x00);
                vga_put(' ',x,y);
                vga_set_col(0xAA);
                x = mmin(x+1,79);
                vga_set_cur_pos2(x,y);
                vga_put(' ',x,y);
            }
            if (prs && ki.key == 'h') {
                vga_set_col(0x00);
                vga_put(' ',x,y);
                vga_set_col(0xAA);
                x = mmax(x-1,0);
                vga_set_cur_pos2(x,y);
                vga_put(' ',x,y);
            }
            if (rls && ki.key == Enter_Key) {
                for (int i = 0; i < particle_num; i++) {
                    if (sands[i].x < 0) {
                        sands[i].x = x;
                        sands[i].y = y;
                        sands[i].z = ptimer;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < particle_num; ++i) {
            if (sands[i].x <= 0 || sands[i].x >= 79) { sands[i].x = -1; continue; }
            if (sands[i].y >= 24) {
                sands[i].x = -1;
                continue;
            }
            if (sands[i].z-- > 0) continue;
            sands[i].z = ptimer;
            
            int r = (vga_get_value2(sands[i].x+1, sands[i].y+1)) >> 8;
            int l = (vga_get_value2(sands[i].x-1, sands[i].y+1)) >> 8;
            int d = (vga_get_value2(sands[i].x, sands[i].y+1)) >> 8;

            i2  off; off.x = 0; off.y = 1;
            if (d == 0x20)     off.y = 1;
            else if(l == 0x20) off.x = -1; 
            else if(r == 0x20) off.x = 1; 
            else {sands[i].x = -1; continue;}

            vga_set_col(0x00);
            vga_put(' ',sands[i].x, sands[i].y);
            vga_set_col(0xAA);
            sands[i].x += off.x;
            sands[i].y += off.y;
            vga_put(' ',sands[i].x,sands[i].y);
        }
        vga_update_gcur();
    }
}
//-------------------------------


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

    //basic_text_editor();
    //vga_set_col(0xAA);
    vga_put('X', 0, 0);
    vga_set_cur_pos2(10,3);
    vga_update_gcur();
    //vga_print16(vga_get_value2(10, 9));
    sand_gui();
}
