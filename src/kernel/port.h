#ifndef PORT_H
#define PORT_H
#include "globals.h"

ui8  p_in  (ui16 port);
ui16 p_in16(ui16 port);
ui32 p_in32(ui16 port);

void p_out  (ui16 port, ui8 data);
void p_out16(ui16 port, ui16 data);
void p_out32(ui16 port, ui32 data);


#endif
