#ifndef VIDEO_H
#define VIDEO_H
#include "globals.h"

#define VGA_VIDEO_MEMORY  (0xB8000)


//----------------------VGA-----------------------
typedef struct  {
    int cur; 
    int width;
    ui16 col;
} VGA_Data;

extern VGA_Data  vga_def_vd;
extern VGA_Data* vga_ptr_vd;


ui16 vga_get_value(int p);
ui16 vga_get_value2(int x, int y);
void vga_get_cur_pos2(int* x, int* y);
void vga_set_cur_pos2(int x, int y);
int vga_get_cur_pos();
void vga_get_cur_pos2(int* x, int* y);
void vga_set_col(ui16 col);
int vga_put(char c, int x, int y);
void vga_set_gcur_pos(int p);
void vga_fill_till_cur(char c);
int vga_print_char(char c);
void vga_clear();
int vga_print_char(char c);
void vga_update_gcur();
void vga_set_gcur_pos2(int x, int y);
void vga_get_gcur_pos2(int* x, int* y);
int vga_get_gcur_pos();
int vga_print(const char* str);
void print_hex(int n, int d);
void vga_go_next_line();
void vga_print16(int n);
void vga_print32(int n);
void vga_query_cur();
void vga_query_width();
void drv_init_vga();

//------------------------------------------------
// https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial
// https://wiki.osdev.org/VESA_Video_Modes
typedef struct {
    char    signature[4];
    ui16    version;
    ui32    oem;
    ui32    cap;
    ui32    vid_modes;
    ui16    vid_mem;
    ui16    soft_rev;
    ui32    vendor;
    ui32    prod_name;
    ui32    prod_rev;
    char    _res[222];
    char    _oem_data[256];
} __attribute__((packed)) vbe_info;

#endif
