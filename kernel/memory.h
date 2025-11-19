#ifndef  MEMORY_H
#define MEMORY_H 
#include "globals.h"

#define PD_PRES      1
#define PD_WRT      (1<<1)
#define PD_USER     (1<<2)
#define PD_FMB      (1<<7)

#define PT_PRES      1
#define PT_WRT      (1<<1)

typedef ui32 pg_dir_entry_t;
typedef ui32 pg_tab_entry_t;

void pg_init_dir();
void pg_map_iden(ui32 addr);
void pg_make_dir_entry(ui32 tab, ui32 idx);
void pg_map(ui32 phy_addr, ui32 v_addr);
void pg_init_krn_tabs();
void pg_init_fmbuff_tabs();
void pg_map_vbe_fmbuff();
void pg_enable();
void pg_init();

#endif
