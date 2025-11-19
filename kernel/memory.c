#include "memory.h"
#include "video.h"


static pg_dir_entry_t pg_dir[1024]             __attribute__((aligned(4096)));
static pg_tab_entry_t pg_first_tabs[1024*4]    __attribute__((aligned(4096))); //16MB
static pg_tab_entry_t pg_fmbuff_tabs[1024*4]   __attribute__((aligned(4096))); //2000x2000x4 is assumed as max metrics


void pg_init_dir() {
    ui32 val;
    vbe_mode_info* vinf = vbe_get_mode_info(); 
    ui32 fmbuff  = vinf->framebuffer;

    for (int i = 0; i < 1024; pg_dir[i++] = 0);
    for (int i = 0; i < 4; ++i) {
        pg_make_dir_entry((ui32)pg_first_tabs+i*0x1000, i);
    }
    for (int i = 0; i < 4; ++i) {
        pg_make_dir_entry((ui32)pg_fmbuff_tabs+i*0x1000, ((fmbuff + i*0x400000) >> 22) & 0x3FF);
    }
}

void pg_map_iden(ui32 addr) {
    pg_map(addr, addr);
}

void pg_make_dir_entry(ui32 tab, ui32 idx) {
    pg_dir[idx] = (tab & 0xFFFFF000) | PD_PRES | PD_WRT;
}

void pg_map(ui32 phy_addr, ui32 v_addr) {
    ui32  dir_i     = (v_addr >> 22) & 0x3FF;
    ui32  tab_i     = (v_addr >> 12) & 0x3FF;
    ui32* tab       = (ui32*)(pg_dir[dir_i] & 0xFFFFF000);
    tab[tab_i]      = (phy_addr & 0xFFFFF000) | PD_PRES | PD_WRT; 
}

void pg_init_krn_tabs() {
    for (int i = 0; i < 1024 * 4; ++i) {
        pg_first_tabs[i] = (i * 0x1000) | PT_PRES | PT_WRT;
    }
}

void pg_init_fmbuff_tabs() {
    vbe_mode_info* vinf = vbe_get_mode_info(); 
    ui32 fmbuff  = vinf->framebuffer;
    for (int i = 0; i < 1024 * 4; ++i) {
        pg_fmbuff_tabs[i] = ((fmbuff + (i * 0x1000)) & 0xFFFFF000) | PT_PRES | PT_WRT;
    }
}

void pg_map_vbe_fmbuff() {
    ui32 i;
    ui32 id_addr;
    vbe_mode_info* vinf = vbe_get_mode_info(); 
    ui32 fmbuff  = vinf->framebuffer;
    ui32  size   = vbe_get_fmbuff_size();
    ui32  pgc    = (size + 0xFFF) >> 12; //ceil(size/4096)
    for (i = 0; i < pgc; ++i) {
        id_addr = fmbuff + i*0x1000;
        pg_map(id_addr, id_addr);
    }
}

void pg_enable() {
    ui32 val = 0;
    __asm__ volatile("mov %0, %%cr3" :: "r"(pg_dir));
    __asm__ volatile("mov %%cr0, %0" :  "=r"(val));
    val |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(val));
}   

void pg_init() {
    pg_init_krn_tabs();
    pg_init_fmbuff_tabs();
    pg_init_dir();
    pg_enable();
}

