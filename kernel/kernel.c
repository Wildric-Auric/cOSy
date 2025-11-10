#include "inter.h"
#include "util.h"
#include "kb.h"
#include "video.h"
#include "port.h"

volatile ui8 tick;
void timer_isr_cbk(isr_hdl_args arg) {
    ++tick;
    //vga_clear();
    //vga_print32(tick);
}
void drv_init_timer(ui32 frq) {
    bind_int_hdl(32, timer_isr_cbk);
    ui32 d = 1193180 / frq;
    p_out(0x43, 0x36);
    p_out(0x40, d & 0xFF);
    p_out(0x40, (d>>8) & 0xFF);
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
#define ptimer 100
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
ui16 glob = 42;
void test_vga() {
    vga_put('X', 0, 0);
    vga_set_cur_pos2(10,3);
    vga_update_gcur();
    vga_set_col(0xAA);
    vga_print16(vga_get_value2(10, 9));
    vga_go_next_line();
    int x,y;
    vga_set_col(0x0F);
    vga_get_cur_pos2(&x,&y);
    vga_set_cur_pos2(0,y);
    vga_update_gcur();  
    vga_print16(glob);
    vga_go_next_line();
    //00003af0
    vga_go_next_line();
    vga_print(a);
    //vga_print(a);
}

void print_line() {
    vga_go_next_line();
    for (int i = 0; i < vga_ptr_vd->width; ++i) {
        vga_print("-"); 
    }
    vga_go_next_line();
}

extern ui8 sys_font[128][16];



extern void init_sys_font();
int main() {
    drv_init_vga();
    vga_clear();
    set_idt();
    asm __volatile__("sti");
    drv_init_timer(100);
    drv_init_kb(); 
    init_sys_font();
    //vbe_put_char('c',&ctx);
    vbe_mode_info* mi = vbe_get_mode_info();
    vbe_txt_cnst  c;
    c.end_loc       = 500;
    c.next_line_pos = 0;

    vbe_line_info li;
    li.level = c.end_loc;
    li.width = 1;
    li.col.x = 100; li.col.y = 100; li.col.z = 150;
    vbe_draw_line_vr(&li);

    ui8 tt = 0;
    vbe_txt_ctx ctx;
    vbe_txt_ctx bct; bct.size.x = 4; bct.size.y = 4; 
    ctx.size.x = 2;
    ctx.size.y = 2;
    vbe_init_ctx(&ctx, &ctx.size); ctx.gap.x = 0; ctx.gap.y = 0;
    vbe_init_ctx(&bct, &bct.size); bct.gap.x = 0; bct.gap.y = 0;
    bct.bcol.x = 0; bct.bcol.y = 0; bct.bcol.z = 255;
    bct.col.x  = 100.0; bct.col.y = 0; bct.col.z = 0; 
    vbe_put_str("Be Right Back!", &bct);
    vbe_go_next_line(&bct); bct.cur.x = 0;
    ctx.cur = bct.cur;
    ctx.bcol.x = 0; ctx.bcol.y = 0; ctx.bcol.z = 0;
    ctx.col.x = 0; ctx.col.z = 0;
    vbe_display_info(&ctx);
    //vbe_put_str_check("Wake up neo,the matrix has you", &ctx, &c);

    while (1) {
        bct.col.x  = tt*10.0; bct.col.y = 0; bct.col.z = 0; 
        bct.cur.x  = 0;
        bct.cur.y  = 0;
        vbe_put_str("Be Right Back!", &bct);
        vbe_go_next_line(&bct); bct.cur.x = 0;
        ctx.cur = bct.cur;
        vbe_display_info(&ctx);
        tt++;
    }

    while (1){};
    return 0;
}
