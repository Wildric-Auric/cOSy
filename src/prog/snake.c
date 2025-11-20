#include "video.h"
#include "kb.h"

typedef struct  {
    ui8 pos_x;
    ui8 pos_y;
    ui8 dir;
} snake_t;

static snake_t snake_body[256];

static void snake_render_part(int x, int y, i3* col) {
    i2 pos;
    for (pos.x = x*10; pos.x < x*10+10; ++pos.x) {
    for (pos.y = y*10; pos.y < y*10+10; ++pos.y) {
        vbe_put_pxl(&pos,col);
    }}
}

void snake() {
    vbe_mode_info* inf = vbe_get_mode_info();
    i2      pos;
    i3      bg;
    i3      bg1;
    i3      scol;
    ui16    sc;
    key_inf ki;
    snake_t* cur;
    i2      lpos; 
    int     len  = 0;
    bool    quit = 0;
    int     idx  = 0;
    int     head_dir;
    int     tmr  = 10e6;
    int     tick = tmr;
    make_vec3(scol, 255, 0, 0);
    bg.x  = 80; bg.y = 0; bg.z = 50;
    bg1.x = 150; bg1.y = 90; bg1.z = 0;
    for (pos.x = 0; pos.x < inf->width; ++pos.x) {
    for (pos.y = 0; pos.y < inf->height; ++pos.y) {
        if (pos.x < 500 && pos.y < 500)
            vbe_put_pxl(&pos,&bg1);
        else
            vbe_put_pxl(&pos,&bg);
    }}

    len = 2;
    for (int i = 0; i < len; i++) {
        snake_body[i].dir   = 1;
        snake_body[i].pos_x = 5 - i;
        snake_body[i].pos_y = 10;
    }
    int tmp_dir = 0;
    while (!quit) {
        while (kb_get_stack_ptr()) {
            sc   = kb_pop_scode();
            kb_cnv_scode(sc, &ki);
            bool ev = ki.event == key_event_enm_released;
            if (ki.key == 'a' && ev) {
                tmp_dir = 0x2;  
            }
            else if (ki.key == 'd' && ev) {
                tmp_dir = 0x01;
            }
            else if (ki.key == 's' && ev) {
                tmp_dir = 0x04;
            }
            else if (ki.key == 'w' && ev) {
                tmp_dir = 0x08;
            }
            if (ki.key != 'c') 
                continue;
            quit = 1;
            break;
        }  
        tick--;
        if (tick) continue;
        tick = tmr;
        idx  = 0;
        cur  = 0;
        make_vec2(lpos,snake_body[len-1].pos_x, snake_body[len-1].pos_y);
        for (idx = 0; idx < len; ++idx) {
            cur = &snake_body[idx];
            cur->pos_x += ((cur->dir & 0x1)      - ((cur->dir & 0x2)>>1));
            cur->pos_y += (((cur->dir & 0x4)>>2) - ((cur->dir & 0x8)>>3));
            snake_render_part(cur->pos_x,cur->pos_y,&scol);
        }
        snake_render_part(lpos.x, lpos.y, &bg1);
        head_dir = snake_body[0].dir;
        if (tmp_dir == 0 || tmp_dir == head_dir) 
            continue;
        if (tmp_dir == 0x2 && head_dir == 0x1) 
            continue;
        if (tmp_dir == 0x1 && head_dir == 0x2) 
            continue;
        if (tmp_dir == 0x4 && head_dir == 0x8) 
            continue;
        if (tmp_dir == 0x8 && head_dir == 0x4) 
            continue;
        for (int i = 1; i < len; ++i ) {
            snake_body[i].dir = snake_body[i-1].dir;
        }
        snake_body[0].dir = tmp_dir;
        
    }
}

