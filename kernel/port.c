#include "port.h"

ui8 p_in(ui16 port) {
    ui8 res; 
    __asm__("in %%dx, %%al" : "=a" (res) : "d" (port));
    return res;
}
void p_out(ui16 port, ui8 data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}
ui32 p_in32(ui16 port) {
    ui32 res; 
    __asm__("in %%dx, %%eax" : "=a" (res) : "d" (port));
    return res;
}
void p_out32(ui16 port, ui32 data) {
    __asm__("out %%eax, %%dx" : : "a" (data), "d" (port));
}
ui16 p_in16(ui16 port) {
    ui16 res; 
    __asm__("in %%dx, %%ax" : "=a" (res) : "d" (port));
    return res;
}
void p_out16(ui16 port, ui16 data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}


