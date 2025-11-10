#include "kb.h"
#include "inter.h"
#include "port.h"

//wiki.osdev.org/index.php?title=I8042_PS/2_Controller
//wiki.osdev.org/PS/2_Keyboard#Scan_Code_Sets


ui16 kb_stack[256];
ui8 kb_stack_ptr;

ui8 kb_get_stack_ptr() {
    return kb_stack_ptr;
}

ui16 kb_pop_scode() {
    if (!kb_stack_ptr) return 0;
    return kb_stack[--kb_stack_ptr];
}

ui16 kb_peek_scode() {
    if(!kb_stack_ptr) return 0;
    return kb_stack[kb_stack_ptr-1];
}

void kb_cbk() {
    ui8  scancode            = p_in(0x60);
    ui16 ev                  = (scancode >= 0x80) ? 0x2 : 0x1; //if bigger than 0x80 its release
    ui16 nsc =  scancode - (ev & key_event_enm_released ?  0x80 : 0x0);
    kb_stack[kb_stack_ptr++] = (ev << 8)|(nsc);
}

void drv_init_kb() {
    kb_stack_ptr = 0;
    bind_int_hdl(33, kb_cbk);
    for (int i = 0; i < 256; kb_stack[i++]=0);
}

void kb_cnv_scode(ui16 scode, key_inf* out) {
    out->event = scode >> 8;
    ui8 scan_code = scode & 0xFF;
    switch(scan_code) {
        case 0x00: out->key = 0; break;
        case 0x01: out->key = ESC_Key; break;
        case 0x02: out->key = '1'; break;
        case 0x03: out->key = '2'; break;
        case 0x04: out->key = '3'; break;
        case 0x05: out->key = '4'; break;
        case 0x06: out->key = '5'; break;
        case 0x07: out->key = '6'; break;
        case 0x08: out->key = '7'; break;
        case 0x09: out->key = '8'; break;
        case 0x0A: out->key = '9'; break;
        case 0x0B: out->key = '0'; break;
        case 0x0C: out->key = '-'; break;
        case 0x0D: out->key = '='; break;
        case 0x0E: out->key = Back_Key; break;
        case 0x0F: out->key = Tab_Key; break;
        case 0x10: out->key = 'q'; break;
        case 0x11: out->key = 'w'; break;
        case 0x12: out->key = 'e'; break;
        case 0x13: out->key = 'r'; break;
        case 0x14: out->key = 't'; break;
        case 0x15: out->key = 'y'; break;
        case 0x16: out->key = 'u'; break;
        case 0x17: out->key = 'i'; break;
        case 0x18: out->key = 'o'; break;
        case 0x19: out->key = 'p'; break;
        case 0x1A: out->key = '['; break;
        case 0x1B: out->key = ']'; break;
        case 0x1C: out->key = Enter_Key; break;
        case 0x1D: out->key = LCtrl_Key; break;
        case 0x1E: out->key = 'a'; break;
        case 0x1F: out->key = 's'; break;
        case 0x20: out->key = 'd'; break;
        case 0x21: out->key = 'f'; break;
        case 0x22: out->key = 'g'; break;
        case 0x23: out->key = 'h'; break;
        case 0x24: out->key = 'j'; break;
        case 0x25: out->key = 'k'; break;
        case 0x26: out->key = 'l'; break;
        case 0x27: out->key = ';'; break;
        case 0x28: out->key = '\''; break;
        case 0x29: out->key = '`'; break;
        case 0x2A: out->key = LShift_Key; break;
        case 0x2B: out->key = '\\'; break;
        case 0x2C: out->key = 'z'; break;
        case 0x2D: out->key = 'x'; break;
        case 0x2E: out->key = 'c'; break;
        case 0x2F: out->key = 'v'; break;
        case 0x30: out->key = 'b'; break;
        case 0x31: out->key = 'n'; break;
        case 0x32: out->key = 'm'; break;
        case 0x33: out->key = ','; break;
        case 0x34: out->key = '.'; break;
        case 0x35: out->key = '/'; break;
        case 0x36: out->key = RShift_Key; break;
        case 0x37: out->key = '*'; break;
        case 0x38: out->key = Alt_Key; break;
        case 0x39: out->key = Space_Key; break;
        case 0x3A: out->key = Caps_Key; break;
        case 0x3B: out->key = F1_Key; break;
        case 0x3C: out->key = F2_Key; break;
        case 0x3D: out->key = F3_Key; break;
        case 0x3E: out->key = F4_Key; break;
        case 0x3F: out->key = F5_Key; break;
        case 0x40: out->key = F6_Key; break;
        case 0x41: out->key = F7_Key; break;
        case 0x42: out->key = F8_Key; break;
        case 0x43: out->key = F9_Key; break;
        case 0x44: out->key = F10_Key; break;
        case 0x57: out->key = F11_Key; break;
        case 0x58: out->key = F12_Key; break;
        default: out->key = 0; break;
    }
}
