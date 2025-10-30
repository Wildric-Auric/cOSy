
typedef unsigned char  uchar;
typedef unsigned short ushort;

#define VIDEO_MEMORY  0xb8000


typedef struct  {
    int cur; 
    int width;
} TTY_Data;

TTY_Data default_td = {};

uchar rd_port_byte(ushort port) {
    uchar res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}
void wrt_port_byte(ushort port, uchar data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

int print_str_tty(const char* str, uchar col,TTY_Data* tty) {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    char c;
    int  i = 0;
    while (c = str[i]) {
        *(vm+tty->cur)     = c;
        *(vm+tty->cur+1)   = col;
        tty->cur += 2;
        i++;
    }
    return tty->cur;
}

void fill_till_cur_tty(char c, char col, TTY_Data* td) {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < td->cur; i+=2) {
        *(vm+i) = c;
        *(vm+i) = col;
    }
}

void clear_tty(TTY_Data* td) {
    fill_till_cur_tty(' ', 0x00, &default_td);
    td->cur = 0;
}

void clear() {
    clear_tty(&default_td);
}

int print_str(const char* str) {
    return print_str_tty(str, 0xDF, &default_td);
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

void main() {
    init_td(&default_td);
    clear();
    for (int i = 0; i < default_td.width; ++i) {
        print_str_tty("-",0x0F, &default_td); }
    print_str("Hey there");
    print_str(", From C!");
}
