#ifndef VIDEO_H
#define VIDEO_H
#include "globals.h"

#define VGA_VIDEO_MEMORY         (0xB8000)
#define VBE_MODE_INFO            (0x8600+512)
#define VBE_MODE_INFO_FMBUFF     (0x8600+512+0x28)
#define VBE_MODE_INFO_WIDTH      (0x8600+512+0x12)
#define VBE_MODE_INFO_HEIGHT     (0x8600+512+0x12)

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
void print_hex(ui32 n, ui32 d);
void vga_go_next_line();
void vga_print8(ui32);
void vga_print16(ui32);
void vga_print32(ui32);
void vga_query_cur();
void vga_query_width();
void drv_init_vga();

//---------------------VBE Data--------------------------
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

typedef struct vbe_mode_info_structure {
	ui16 attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	ui8 window_a;			// deprecated
	ui8 window_b;			// deprecated
	ui16 granularity;		// deprecated; used while calculating bank numbers
	ui16 window_size;
	ui16 segment_a;
	ui16 segment_b;
	ui32 win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	ui16 pitch;			// number of bytes per horizontal line
	ui16 width;			// width in pixels
	ui16 height;			// height in pixels
	ui8 w_char;			// unused...
	ui8 y_char;			// ...
	ui8 planes;
	ui8 bpp;			// bits per pixel in this mode
	ui8 banks;			// deprecated; total number of banks in this mode
	ui8 memory_model;
	ui8 bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	ui8 image_pages;
	ui8 reserved0;

	ui8 red_mask;
	ui8 red_position;
	ui8 green_mask;
	ui8 green_position;
	ui8 blue_mask;
	ui8 blue_position;
	ui8 reserved_mask;
	ui8 reserved_position;
	ui8 direct_color_attributes;

	ui32 framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	ui32 off_screen_mem_off;
	ui16 off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	ui8  reserved1[206];
} __attribute__((packed)) vbe_mode_info;

typedef struct {
    i2 cur;
    f2 size;
    i2 gap;
    i3 col;
    i3 bcol;
} vbe_txt_ctx;

typedef struct {
    i32 end_loc;  
    int next_line_pos;
} vbe_txt_cnst;

typedef struct {
    i2  src_dst;
    int level;
    int width;
    i3  col;
} vbe_line_info;

//to test when vga is enabled
//------------------------
void vbe_init_ctx(vbe_txt_ctx* ctx, f2* size);
void vbe_init_ctx_def(vbe_txt_ctx* ctx);
void vbe_put_pxl(i2* pos, i3* col);
void vbe_test_fill();
void vbe_test_eclipse();
void vbe_put_char(char c, vbe_txt_ctx*);
void vbe_put_str(const char*, vbe_txt_ctx*);
int  vbe_put_str_to(const char* str,int n, vbe_txt_ctx* ctx);
void vbe_put_str_check(const char*, vbe_txt_ctx*, vbe_txt_cnst*);
void vbe_draw_line_hz_ft(vbe_line_info*);
void vbe_draw_line_vr_ft(vbe_line_info*);
void vbe_draw_line_hz(vbe_line_info*);
void vbe_draw_line_vr(vbe_line_info*);
void vbe_go_next_line(vbe_txt_ctx*);
void vbe_go_next_line_rewind(vbe_txt_ctx*, int);
vbe_mode_info* vbe_get_mode_info(); 
#endif
