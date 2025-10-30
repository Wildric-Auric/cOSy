
typedef unsigned char  uchar;
typedef unsigned short ushort;

#define VIDEO_MEMORY  0xb8000
volatile int tty_cur = 0;
int print_str(const char* str) {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    char c;
    int  i = 0;
    while (c = str[i]) {
        *(vm+tty_cur)     = c;
        *(vm+tty_cur+1)    = 0xA0;
        tty_cur += 2;
        i++;
    }
    return tty_cur;
}

uchar rd_port_byte(ushort port) {
    uchar res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}

void wrt_port_byte(uchar port, uchar data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

void main() {
    print_str("Hey there");
    print_str("; From C!");
}
