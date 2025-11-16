#ifndef UTIL_H
#define UTIL_H
#include "globals.h"

char    cnv_ascii_qwaz(char c);
bool    is_alpha_num(char c);
void    cnv_num_hex_str(ui32 n, ui32 d, char* out_buff);
void    cnv_lba_to_chs(ui32 lba, ui32 secPerCyl, ui32* cyl, ui32* head, ui32* sec);
#endif
