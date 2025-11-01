#include "globals.h"


#define VIDEO_MEMORY  0xb8000

typedef struct  {
    int cur; 
    int width;
} TTY_Data;

TTY_Data default_td={};

uchar rd_port_byte(ushort port) {
    uchar res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}
void wrt_port_byte(ushort port, uchar data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

int print_char(char c, uchar col,TTY_Data* tty) {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    *(vm+tty->cur)     = c;
    *(vm+tty->cur+1)   = col;
    tty->cur += 2;
    return tty->cur;
}

int print_str_tty(const char* str, uchar col,TTY_Data* tty) {
    char c = 0;
    int  i = 0;
    while (c = str[i++]) {
        print_char(c, col, tty);
    }
    return tty->cur;
}

void fill_till_cur_tty(char c, char col, TTY_Data* td) {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < td->cur; i+=2) {
        *(vm+i)   = c;
        *(vm+i+1) = col;
    }
}

void clear_tty(TTY_Data* td) {
    fill_till_cur_tty(' ', 0x00, &default_td);
    td->cur = 0;
}

void clear() {
    clear_tty(&default_td);
}

void set_cursor_pos(int p) {
    wrt_port_byte(0x3D4, 0xF);
    wrt_port_byte(0x3D5, p & 0xFF);
    wrt_port_byte(0x3D4, 0xE);
    wrt_port_byte(0x3D5, (p >> 8) & 0xFF);
}

void set_cursor_pos2(int x, int y) {
    set_cursor_pos(default_td.width * y + x);
}



void get_cursor_pos2(int* x, int* y) {
    wrt_port_byte(0x3D4, 0x0F); 
    *x = rd_port_byte(0x3D5);
    wrt_port_byte(0x3D4, 0x0E); 
    *y = rd_port_byte(0x3D5);
}

int get_cursor_pos() {
    int x,y=0;
    get_cursor_pos2(&x,&y);
    return (y << 8) | x;
}

int print_str(const char* str) {
    return print_str_tty(str, 0x0F, &default_td);
}

void print_hex(int n, int d) {
    const char* lkup = "0123456789ABCDEF";
    int r = 0;
    int t = 0;
    print_str("0x");
    do {
        t = n / d; 
        r = n % d;
        print_char(lkup[t], 0x0F, &default_td);
        n = r;
        d >>= 4;
    } while (n);
}
void tty_go_next_line() {
    default_td.cur += ((default_td.width - (default_td.cur/2) % default_td.width) * 2);
}

void print_hex16(int n) {
    print_hex(n, 0x1000);
}
void print_hex32(int n) {
    print_hex(n, 0x10000000);
}

void query_tty_cur(TTY_Data* td) {
    wrt_port_byte(0x3D4, 14); //request high byte of cursor pos, returned in vga data register
    td->cur = rd_port_byte(0x3D5) << 8;
    wrt_port_byte(0x3D4, 15); //low byte
    td->cur |= rd_port_byte(0x3D5); 
    td->cur *= 2;
}

void query_tty_width(TTY_Data* td) {
    wrt_port_byte(0x3D4, 1); 
    td->width = rd_port_byte(0x3D5) + 1;
}

void init_td(TTY_Data* td) {
    td->cur   = 0;
    td->width = 0;
    query_tty_cur(td);
    query_tty_width(td);
}

void set_idt();

void main() {
    init_td(&default_td);
    clear();
    set_idt();

    int x = 10;
    x=x/0; //interrupt triggered
          
    tty_go_next_line();
    for (int i = 0; i < default_td.width; ++i) {
        print_str_tty("-",0x0F, &default_td); 
    }
    //const char* a = "Hey there";
    tty_go_next_line();
    print_str("Hey there");
    print_str(", From C!");
    set_cursor_pos((default_td.cur-1) >> 1);  
}
