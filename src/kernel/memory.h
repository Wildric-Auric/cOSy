#ifndef MEMORY_H
#define MEMORY_H
#include "globals.h"

//Page directory flags
#define PD_PRES      1
#define PD_WRT      (1<<1)
#define PD_ACCESS   (1<<2)
#define PD_PWT      (1<<3)
#define PD_PCD      (1<<4)
#define PD_PG_SIZE  (1<<7)

//Page table flags
#define PT_PRES      1
#define PT_WRT      (1<<1)

void pg_dir_init();
void pg_dir_enable_at(ui32* pg_dir, ui32 idx, ui32 pg_tab);
void pg_tab_enable_at(ui32* pg_tab, ui32 idx, ui32 addr);
void pg_enable();
void pg_init();

#endif
