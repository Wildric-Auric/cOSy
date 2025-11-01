#ifndef KB_H
#define KB_H

#include "globals.h"

enum key_event_enm {
    key_event_enm_pressed  = 0x1,
    key_event_enm_released = 0x2
};

typedef struct {
    ui8 key;
    ui8 event;
} __attribute__((packed)) key_inf;


void drv_init_kb();
ui8  kb_get_stack_ptr();
ui16 kb_pop_scode();
ui16 kb_peek_scode();
void kb_cnv_scode(ui16 scode, key_inf* out);


#define ESC_Key     128
#define F1_Key      129
#define F2_Key      130
#define F3_Key      131
#define F4_Key      132
#define F5_Key      133
#define F6_Key      134
#define F7_Key      135
#define F8_Key      136
#define F9_Key      137
#define F10_Key     138
#define F11_Key     139
#define F12_Key     140
#define LShift_Key  141
#define RShift_Key  142
#define LCtrl_Key   143
#define Alt_Key     144
#define Space_Key   145
#define Enter_Key   146
#define Back_Key    147
#define Tab_Key     148
#define Caps_Key    149


#endif
