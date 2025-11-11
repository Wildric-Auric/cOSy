#include "util.h"

char cnv_ascii_qwaz(char c) {
    switch(c) {
        case 'a': return 'q'; case 'q': return 'a'; case 'w': return 'z'; case 'z': return 'w';
        case 'A': return 'Q'; case 'Q': return 'A'; case 'W': return 'Z'; case 'Z': return 'W';
        case 'm': return ','; case ',': return 'm'; case 'M': return '?'; case '?': return 'M';
        case ';': return 'm'; case ':': return 'M'; case '.': return ';';
        default: return c;
    }
}

bool is_alpha_num(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

void cnv_num_hex_str(ui32 n, ui32 d, char* out_buff) {
    const char* lkup = "0123456789ABCDEF";
    int r = 0;
    int t = 0;
    int l = 0;
    out_buff[0] = '0'; out_buff[1] = 'x';
    int i   = 2;
    ui32 digit = 5;
    while (d) {
        digit      = mmin(n / d,15);       
        out_buff[i++]  = lkup[digit];
        n %= d;
        d  = (~d)&(d>>4);
    }
    out_buff[i] = 0;
}

int str_len(const char* str) {
    char c;
    int  i = 0;
    while (c = str[i++]);
    return i;
}

