#ifndef UTIL_H
#define UTIL_H
#include "globals.h"

char    cnv_ascii_qwaz(char c);
bool    is_alpha_num(char c);
void    cnv_num_hex_str(ui32 n, ui32 d, char* out_buff);

#endif
