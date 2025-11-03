#include "video.h"
extern uchar rd_port_byte(ushort port);
extern void  wrt_port_byte(ushort port, uchar data);

VGA_Data  vga_def_vd = {};
VGA_Data* vga_ptr_vd = &vga_def_vd;


ui16 vga_get_value(int p) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    ui16 h = *(vm+p);
    ui16 l = *(vm+p+1);
    return (h << 8) | (l);
}

ui16 vga_get_value2(int x, int y) {
    return vga_get_value((y * vga_ptr_vd->width + x)*2);
}

void vga_set_col(ui16 col) {
    vga_ptr_vd->col = col;
}

int vga_put(char c, int x, int y) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    int p = (y * vga_ptr_vd->width + x)*2;
    *(vm+p)   = c;
    *(vm+p+1) = vga_ptr_vd->col;
    return p;
}

int vga_print_char(char c) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    *(vm+vga_ptr_vd->cur)   = c;
    *(vm+vga_ptr_vd->cur+1) = vga_ptr_vd->col;
    vga_ptr_vd->cur += 2;
    vga_update_gcur();
    return vga_ptr_vd->cur;
}

void vga_fill_till_cur(char c) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    for (int i = 0; i < vga_ptr_vd->cur; i+=2) {
        *(vm+i)   = c;
        *(vm+i+1) = vga_ptr_vd->col;
    }
}

void vga_clear() {
    vga_fill_till_cur(' ');
    vga_ptr_vd->cur = 0;
}

void vga_set_gcur_pos(int p) {
    wrt_port_byte(0x3D4, 0xF);
    wrt_port_byte(0x3D5, p & 0xFF);
    wrt_port_byte(0x3D4, 0xE);
    wrt_port_byte(0x3D5, (p >> 8) & 0xFF);
}

void vga_update_gcur() {
    vga_set_gcur_pos((vga_ptr_vd->cur-1) >> 1);  
}

void vga_set_gcur_pos2(int x, int y) {
    vga_set_gcur_pos(vga_ptr_vd->width * y + x);
}


int vga_get_cur_pos() {
    return vga_ptr_vd->cur / 2;
}

void vga_get_cur_pos2(int* x, int* y) {
    int p = vga_get_cur_pos();
    *y = p / vga_ptr_vd->width; 
    *x = p % vga_ptr_vd->width; 
}

void vga_set_cur_pos(int p) {
    vga_ptr_vd->cur = p * 2;
}

void vga_set_cur_pos2(int x, int y) {
    vga_set_cur_pos(y * vga_ptr_vd->width + x);
}

void vga_get_gcur_pos2(int* x, int* y) {
    wrt_port_byte(0x3D4, 0x0F); 
    *x = rd_port_byte(0x3D5);
    wrt_port_byte(0x3D4, 0x0E); 
    *y = rd_port_byte(0x3D5);
}

int vga_get_gcur_pos() {
    int x,y=0;
    vga_get_gcur_pos2(&x,&y);
    return (y << 8) | x;
}

int vga_print(const char* str) {
    char c = 0;
    int  i = 0;
    while (c = str[i++]) {
        vga_print_char(c);
    }
    return vga_ptr_vd->cur;
}

void print_hex(int n, int d) {
    const char* lkup = "0123456789ABCDEF";
    int r = 0;
    int t = 0;
    vga_print("0x");
    do {
        t = n / d; 
        r = n % d;
        vga_print_char(lkup[t]);
        n = r;
        d >>= 4;
    } while (n);
}
void vga_go_next_line() {
    vga_ptr_vd->cur += ((vga_ptr_vd->width - (vga_ptr_vd->cur/2) % vga_ptr_vd->width) * 2);
}

void vga_print16(int n) {
    print_hex(n, 0x1000);
}
void vga_print32(int n) {
    print_hex(n, 0x10000000);
}

void vga_query_cur() {
    wrt_port_byte(0x3D4, 14); //request high byte of cursor pos, returned in vga data register
    vga_ptr_vd->cur = rd_port_byte(0x3D5) << 8;
    wrt_port_byte(0x3D4, 15); //low byte
    vga_ptr_vd->cur |= rd_port_byte(0x3D5); 
    vga_ptr_vd->cur *= 2;
}

void vga_query_width() {
    wrt_port_byte(0x3D4, 1); 
    vga_ptr_vd->width = rd_port_byte(0x3D5) + 1;
}

void drv_init_vga() { 
    vga_ptr_vd->cur   = 0;
    vga_ptr_vd->width = 0;
    vga_ptr_vd->col   = 0x0F;
    vga_query_cur();
    vga_query_width();
}
