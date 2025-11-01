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
