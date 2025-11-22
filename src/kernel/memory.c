#include "memory.h"
#include "globals.h"

//
#define KERNEL_BEG   0x100000
#define KERNEL_END   0x4FFFFF
//1024 entry for page dir at PG_DIR_TABLE
#define PG_DIR_BEG   0x500000
#define PG_DIR_END   0x5003FF
//entirety of page tables, 1024*1024*4 bytes
#define PG_TAB_BEG   0x501000
#define PG_TAB_END   0x901000
//13MB
#define STACK_BEG    0xE00000 
//15MB
#define STACK_END    0xF00000

void pg_dir_enable_at(ui32* pg_dir, ui32 idx, ui32 pg_tab) {
    pg_dir[idx] = (0xFFFFF000 & pg_tab) | PD_PRES | PD_WRT;
}

void pg_tab_enable_at(ui32* pg_tab, ui32 idx, ui32 addr) {
    pg_tab[idx] = (0xFFFFF000 & addr) | PT_PRES | PT_WRT;
}

void pg_dir_init() {
    ui32* dir_arr = (ui32*)(PG_DIR_BEG);
    for (int i = 0; i < 1024; ++i) {
        pg_dir_enable_at(dir_arr, i, (PG_TAB_BEG+i*0x1000));
    }
}

void pg_tabs_init() {
    ui32 idx;
    ui32* tabs = (ui32*)PG_TAB_BEG;
    for (int tab_i = 0; tab_i < 1024; ++tab_i) {
        for (int entry = 0; entry < 1024; ++entry)  {
            idx = tab_i*1024+entry;
            pg_tab_enable_at(tabs, idx, idx * 0x1000); 
        }
    }
}

void pg_enable() {
    ui32 val = 0;
    ui32* pg_dir = (ui32*)(PG_DIR_BEG);
    __asm__ volatile("mov %0, %%cr3" :: "r"(pg_dir));
    __asm__ volatile("mov %%cr0, %0" :  "=r"(val));
    val |= 0x80010000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(val));
}   

void pg_init() {
    pg_tabs_init();
    pg_dir_init();
    pg_enable();
}
