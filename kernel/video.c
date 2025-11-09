#include "video.h"
#include "util.h"
extern uchar rd_port_byte(ushort port);
extern void  wrt_port_byte(ushort port, uchar data);

VGA_Data  vga_def_vd = {};
VGA_Data* vga_ptr_vd = &vga_def_vd;

ui16 vga_get_value(int p) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    ui16 h = *(vm+p);
    ui16 l = *(vm+p+1);
    return (h << 8) | (l);
}

ui16 vga_get_value2(int x, int y) {
    return vga_get_value((y * vga_ptr_vd->width + x)*2);
}

void vga_set_col(ui16 col) {
    vga_ptr_vd->col = col;
}

int vga_put(char c, int x, int y) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    int p = (y * vga_ptr_vd->width + x)*2;
    *(vm+p)   = c;
    *(vm+p+1) = vga_ptr_vd->col;
    return p;
}

int vga_print_char(char c) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    *(vm+vga_ptr_vd->cur)   = c;
    *(vm+vga_ptr_vd->cur+1) = vga_ptr_vd->col;
    vga_ptr_vd->cur += 2;
    vga_update_gcur();
    return vga_ptr_vd->cur;
}

void vga_fill_till_cur(char c) {
    volatile char* vm = (volatile char*)VGA_VIDEO_MEMORY;
    for (int i = 0; i < vga_ptr_vd->cur; i+=2) {
        *(vm+i)   = c;
        *(vm+i+1) = vga_ptr_vd->col;
    }
}

void vga_clear() {
    vga_fill_till_cur(' ');
    vga_ptr_vd->cur = 0;
}

void vga_set_gcur_pos(int p) {
    wrt_port_byte(0x3D4, 0xF);
    wrt_port_byte(0x3D5, p & 0xFF);
    wrt_port_byte(0x3D4, 0xE);
    wrt_port_byte(0x3D5, (p >> 8) & 0xFF);
}

void vga_update_gcur() {
    vga_set_gcur_pos((vga_ptr_vd->cur-1) >> 1);  
}

void vga_set_gcur_pos2(int x, int y) {
    vga_set_gcur_pos(vga_ptr_vd->width * y + x);
}


int vga_get_cur_pos() {
    return vga_ptr_vd->cur / 2;
}

void vga_get_cur_pos2(int* x, int* y) {
    int p = vga_get_cur_pos();
    *y = p / vga_ptr_vd->width; 
    *x = p % vga_ptr_vd->width; 
}

void vga_set_cur_pos(int p) {
    vga_ptr_vd->cur = p * 2;
}

void vga_set_cur_pos2(int x, int y) {
    vga_set_cur_pos(y * vga_ptr_vd->width + x);
}

void vga_get_gcur_pos2(int* x, int* y) {
    wrt_port_byte(0x3D4, 0x0F); 
    *x = rd_port_byte(0x3D5);
    wrt_port_byte(0x3D4, 0x0E); 
    *y = rd_port_byte(0x3D5);
}

int vga_get_gcur_pos() {
    int x,y=0;
    vga_get_gcur_pos2(&x,&y);
    return (y << 8) | x;
}

int vga_print(const char* str) {
    char c = 0;
    int  i = 0;
    while (c = str[i++]) {
        vga_print_char(c);
    }
    return vga_ptr_vd->cur;
}

void print_hex(ui32 n, ui32 d) {
    const char* lkup = "0123456789ABCDEF";
    int r = 0;
    int t = 0;
    int l = 0;
    vga_print("0x");
    ui32 digit = 5;
    while (d) {
        digit = mmin(n / d,15);       
        vga_print_char(lkup[digit]);
        n %= d;
        d  = (~d)&(d>>4);
    }
}

void vga_go_next_line() {
    vga_ptr_vd->cur += ((vga_ptr_vd->width - (vga_ptr_vd->cur/2) % vga_ptr_vd->width) * 2);
}


void vga_print8(ui32 n) {
    print_hex(n, 0x10);
}
void vga_print16(ui32 n) {
    print_hex(n, 0x1000);
}
void vga_print32(ui32 n) {
    print_hex(n, 0x10000000);
}

void vga_query_cur() {
    wrt_port_byte(0x3D4, 14); //request high byte of cursor pos, returned in vga data register
    vga_ptr_vd->cur = rd_port_byte(0x3D5) << 8;
    wrt_port_byte(0x3D4, 15); //low byte
    vga_ptr_vd->cur |= rd_port_byte(0x3D5); 
    vga_ptr_vd->cur *= 2;
}

void vga_query_width() {
    wrt_port_byte(0x3D4, 1); 
    vga_ptr_vd->width = rd_port_byte(0x3D5) + 1;
}

void drv_init_vga() { 
    vga_ptr_vd->cur   = 0;
    vga_ptr_vd->width = 0;
    vga_ptr_vd->col   = 0x0F;
    vga_query_cur();
    vga_query_width();
}

//----------------

void vbe_vga_display_info() {
    vga_print("--------------");
    vga_go_next_line();
    vbe_mode_info* inf = (vbe_mode_info*)(VBE_MODE_INFO);
    #define pr(s,n,d) \
    vga_print(s); \
    print_hex(n, d); \
    vga_go_next_line();
    pr("width     : " , inf->width         ,0x1<<12);
    pr("height    : " , inf->height        ,0x1<<12);
    pr("bpp       : " , inf->bpp           ,0x1<<4)
    pr("memm      : " , inf->memory_model  ,0x1<<4);
    pr("fmb       : " , inf->framebuffer   ,0x1<<28);
    pr("redp      : " , inf->red_position  ,0x1<<4);
    pr("bluep     : " , inf->blue_position ,0x1<<4);
    pr("greenp    : " , inf->green_position,0x1<<4);
    pr("redmsk    : " , inf->red_mask      ,0x1<<4);
    pr("bluemsk   : " , inf->blue_mask     ,0x1<<4);
    pr("greenmsk  : ", inf->green_mask     ,0x1<<4);
    #undef pr
    vga_print("--------------");
    vga_go_next_line();
}

void vbe_put_pxl(i2* pos, i3* col) {
    vbe_mode_info* inf = (vbe_mode_info*)(VBE_MODE_INFO);
    ui8* fmb = (ui8*)inf->framebuffer;
    ui16 lb  = inf->pitch;
    fmb     += (lb * pos->y + pos->x * 4);
    *fmb     = (ui8)col->z;
    *(fmb+1) = (ui8)col->y;
    *(fmb+2) = (ui8)col->x;
    *(fmb+3) = 255;
}

void vbe_test_fill() {
    vbe_mode_info* inf = (vbe_mode_info*)(VBE_MODE_INFO);
    for (int i = 0; i < inf->width; ++i) {
    for (int j = 0; j < inf->height; ++j) {
        i2 pos = {i,j};
        i3 col = {255,0,0};
        vbe_put_pxl(&pos,&col); 
    }}
}

//-------------------------
#define unreal(x)  ((x) / ((x) + 0.155f) * 1.019f)
static float sdf_box(f2 point, f2 box_rect) {
   float ax = (point.x < 0.0f) ? -point.x : point.x;
   float ay = (point.y < 0.0f) ? -point.y : point.y;
   f2 delta;
   delta.x = ax - box_rect.x;
   delta.y = ay - box_rect.y;
   // max(delta, 0.0)
   float mx0 = (delta.x > 0.0f) ? delta.x : 0.0f;
   float my0 = (delta.y > 0.0f) ? delta.y : 0.0f;
   float len_sq = mx0*mx0 + my0*my0;
   float approx_len = len_sq*10.0;
   float mm = mmax(delta.x, delta.y);
   float mm2 = mmin(mm, 0.0f);
   return approx_len + mm2;
}
void vbe_test_eclipse() {
    vbe_mode_info* inf = (vbe_mode_info*)(VBE_MODE_INFO);
    int w = 1000;
    int h = 1000;
    for (int i = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
        i2 ppos = {i,j};
        i3 pcol = {255,0,0};
        f2 uv  = {(float)i / w, (float)j / h};
        uv.x = i / (float)w;
        uv.y = j / (float)h;
        uv.x -= 0.5f;
        uv.y -= 0.5f;
        f2 uv0 = uv;
        f2 line_offset = { 0.0f, 0.0f };
        f2 pos;
        pos.x = uv.x;
        pos.y = uv.y - 0.00f;
        f2 line_pos;
        line_pos.x = uv.x - 0.0f;
        line_pos.y = uv.y - 0.34f;
        float r = 0.38f;
        //no sqrt, choose a coefficient
        float sqrt_c = 8.0f;
        float pos_len_sq = pos.x*pos.x + pos.y*pos.y;
        float pos_len = pos_len_sq * sqrt_c;
        float l = pos_len / r;
        // n = pos / l avoid div by zero
        f2 n;
        float ll = l != 0.0f ? l : 1000.0f;
        n.x = pos.x / ll;
        n.y = pos.y / ll;
      // color setup
        f3 ecol = { 242.0f/255.0f, 80.0f/255.0f, 29.0f/255.0f };
        f3 col = { 0.0f, 0.0f, 0.0f };
        float d = l; 
        // thresholds
        f2 tresh;
        tresh.x = 0.95f * r;
        tresh.y = 1.0f * r;
        f2 tresh_0;
        tresh_0.x = tresh.x - 0.02f * r;
        tresh_0.y = tresh.x;
        // smoothstep(tresh0.x, tresh0.y, d)
        float edge_0 = tresh_0.x;
        float edge_1 = tresh_0.y;
        float s = 0.0f;
        if (edge_1 != edge_0) {
            float t = (d - edge_0) / (edge_1 - edge_0);
            // clamp 0..1
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            s = t * t * (3.0f - 2.0f * t);
        } 
        else {s = (d >= edge_1) ? 1.0f : 0.0f;}
        // col = smoothstep * vec3(1.0)
        col.x = s;
        col.y = s;
        col.z = s;
        int outside = 0;
        if (d > tresh.x && tresh.y < 1.0f) {
            float tmp   = (tresh.y - tresh.x);
            tmp   = (tmp == 0.0f) ? 0.001f : tmp;
            float coord = (d - tresh.x)/(tmp);
            coord = (coord == 0.0f) ? 0.001f : coord * 0.2f; 
            col.x = ecol.x / coord;
            col.y = ecol.y / coord;
            col.z = ecol.z / coord;
            outside = 1;
        }
        f2 box_rect = { 0.0f, 0.2f };
        float l_rect = sdf_box(line_pos, box_rect);
        float abs_lrect = (l_rect < 0.0f) ? -l_rect : l_rect;
        abs_lrect       = mmax(0.0001f, abs_lrect);
        // col += uv0.y * ecol * 0.007 / abs(lrect)
        float eps = 1e-6f;
        float inv = (abs_lrect < eps) ? 0.0f : (1.0f / abs_lrect);
            col.x += uv0.y * ecol.x * 0.007f * inv;
            col.y += uv0.y * ecol.y * 0.007f * inv;
            col.z += uv0.y * ecol.z * 0.007f * inv;
        float tx = unreal(col.x); if (tx < 0.0f) tx = 0.0f; if (tx > 1.0f) tx = 1.0f; col.x = tx;
        float ty = unreal(col.y); if (ty < 0.0f) ty = 0.0f; if (ty > 1.0f) ty = 1.0f; col.y = ty;
        float tz = unreal(col.z); if (tz < 0.0f) tz = 0.0f; if (tz > 1.0f) tz = 1.0f; col.z = tz;
        i2 pi = {i,j};
        i3 res; 
        res.x = (int)(col.x * 255.0f);
        res.y = (int)(col.y * 255.0f);
        res.z = (int)(col.z * 255.0f);
        vbe_put_pxl(&pi, &res);
    }}
} 
//-------------------------
extern ui8 sys_font[128][16];
extern i2  sys_font_metrics;


void vbe_init_ctx_def(vbe_txt_ctx* ctx) {
    ctx->size.x = 1; ctx->size.y = 1;
    vbe_init_ctx(ctx, &ctx->size);
}

void vbe_init_ctx(vbe_txt_ctx* ctx, i2* size) {
    ctx->cur.x  = 0;
    ctx->cur.y  = 0;
    ctx->col.x  = 255;
    ctx->col.y  = 255;
    ctx->col.z  = 255;
    ctx->bcol.x = 0;
    ctx->bcol.y = 0;
    ctx->bcol.z = 0;
    ctx->size.x = size->x <= 0 ? 1 : size->x;
    ctx->size.y = size->y <= 0 ? 1 : size->y;
    ctx->gap.x  = ctx->size.x*sys_font_metrics.x/4;
    ctx->gap.y  = ctx->size.y*sys_font_metrics.y/4;
}

void vbe_go_next_line(vbe_txt_ctx* ctx) {
    ctx->cur.y += (ctx->size.y * sys_font_metrics.y 
                 + ctx->gap.y);
}

void vbe_display_info(vbe_txt_ctx* ctx) {
    char num[33];
    vbe_mode_info* inf = (vbe_mode_info*)(VBE_MODE_INFO);
    #define pr(s,n,d)           \
    vbe_put_str(s, ctx);       \
    cnv_num_hex_str(n, d, num); \
    vbe_put_str(num, ctx);     \
    vbe_go_next_line(ctx);     \
    ctx->cur.x = 0; 
    pr("width     : " , inf->width         ,0x1<<12);
    pr("height    : " , inf->height        ,0x1<<12);
    pr("bpp       : " , inf->bpp           ,0x1<<4)
    pr("memm      : " , inf->memory_model  ,0x1<<4);
    pr("fmb       : " , inf->framebuffer   ,0x1<<28);
    pr("redp      : " , inf->red_position  ,0x1<<4);
    pr("bluep     : " , inf->blue_position ,0x1<<4);
    pr("greenp    : " , inf->green_position,0x1<<4);
    pr("redmsk    : " , inf->red_mask      ,0x1<<4);
    pr("bluemsk   : " , inf->blue_mask     ,0x1<<4);
    pr("greenmsk  : ", inf->green_mask     ,0x1<<4);
    #undef pr
}

void vbe_put_char(char c, vbe_txt_ctx* ctx) {
    ui8 msk;
    ui8 lc;
    i2  pos;
    i3* col;
    ui32 m = 1 << (sys_font_metrics.x-1);
    for (int l = 0; l < sys_font_metrics.y*ctx->size.y; ++l) {
        lc = sys_font[c][l/ctx->size.y];
    for (int i = 0; i < sys_font_metrics.x*ctx->size.x; ++i) {
        ui8 msk = m >> (i/ctx->size.x);
        pos.y = ctx->cur.y + l;
        pos.x = ctx->cur.x + i;
        col   = (lc & msk) ? &ctx->col : &ctx->bcol;
        vbe_put_pxl(&pos, col);
    }}
    ctx->cur.x += (sys_font_metrics.x * ctx->size.x) + ctx->gap.x;
    //ctx->cur.y += (sys_font_metrics.y * ctx->size.y) + ctx->gap.y;
}

void vbe_put_str(const char* str, vbe_txt_ctx* ctx) {
    char c;
    int  i = 0;
    while (c = str[i++]) {
        vbe_put_char(c, ctx);
    }
}

int vbe_put_str_to(const char* str,int n, vbe_txt_ctx* ctx) {
    char c;
    int  i = 0;
    while ((c = str[i]) && i != n) {
        vbe_put_char(c, ctx);
        i++;
    }
    return (c!=0) * i;
}

//puts a string, when arrives at limit define by constraint, it goes to next line
//if limit is less than character width (*size + gap), nothing is output
void vbe_put_str_check(const char* str, vbe_txt_ctx* ctx, vbe_txt_cnst* cnt) {
    char c;
    i32  i = 0;
    i32 charw = sys_font_metrics.x * ctx->size.x + ctx->gap.x;
    while (c = str[i]) {
        int diff = cnt->end_loc - ctx->cur.x;
        int n    = diff/charw;
        if (!vbe_put_str_to(&str[i], n, ctx))
            break;
        ctx->cur.x  = cnt->next_line_pos;
        ctx->cur.y += (sys_font_metrics.y * ctx->size.y) + ctx->gap.y;
        i += n;
    }
}

void vbe_draw_line_hz_ft(vbe_line_info* inf) {
    i2 pos;
    int j;
    int i = -inf->width;
    do {
       for (j = inf->src_dst.x; j < inf->src_dst.y; ++j) {
            pos.y = inf->level+i; pos.x = j;  
            vbe_put_pxl(&pos, &inf->col);
       }
    } while (++i < inf->width);
}
void vbe_draw_line_vr_ft(vbe_line_info* inf) {
    i2 pos;
    int j;
    int i = -inf->width;
    do {
       for (j = inf->src_dst.x; j < inf->src_dst.y; ++j) {
            pos.x = inf->level+i; pos.y = j;  
            vbe_put_pxl(&pos, &inf->col);
       }
    } while (++i < inf->width);

}
void vbe_draw_line_hz(vbe_line_info* inf) {
    inf->src_dst.x = 0;
    inf->src_dst.y = vbe_get_mode_info()->width;
    vbe_draw_line_hz_ft(inf);
}
void vbe_draw_line_vr(vbe_line_info* inf) {
    inf->src_dst.x = 0;
    inf->src_dst.y = vbe_get_mode_info()->height;
    vbe_draw_line_vr_ft(inf);
}

vbe_mode_info* vbe_get_mode_info() {
    return (vbe_mode_info*)(VBE_MODE_INFO);
}
