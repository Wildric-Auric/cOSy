#ifndef PORT_H
#define PORT_H
#include "globals.h"

ui8  p_in(ui16 port);
void p_out(ui16 port, ui8 data);

#endif
