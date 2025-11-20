#include "inter.h"
#include "kb.h"
#include "video.h"

extern void term();

int main() {
    set_idt();
    asm __volatile__("sti");
    drv_init_kb(); 
    init_sys_font();
    //vbe_test_eclipse();
    term();
    while (1){};
    return 0;
}
