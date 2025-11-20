#include "inter.h"
#include "port.h"

volatile ui8 tick;

void timer_isr_cbk(isr_hdl_args arg) {
    ++tick;
}

void drv_init_timer(ui32 frq) {
    bind_int_hdl(32, timer_isr_cbk);
    ui32 d = 1193180 / frq;
    p_out(0x43, 0x36);
    p_out(0x40, d & 0xFF);
    p_out(0x40, (d>>8) & 0xFF);
}
