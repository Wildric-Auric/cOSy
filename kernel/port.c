#include "port.h"

ui8 p_in(ui16 port) {
    ui8 res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}
void p_out(ui16 port, ui8 data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}


