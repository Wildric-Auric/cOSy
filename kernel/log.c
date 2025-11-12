#include "video.h"
#include "util.h"
#include "pci.h"

void vbe_put_hex16(ui16 n, vbe_txt_ctx* ctx) {
    char str[8];
    cnv_num_hex_str(n, 0x1000, str);
    vbe_put_str(str, ctx);
}
void vbe_put_hex32(ui32 n, vbe_txt_ctx* ctx) {
    char str[16];
    cnv_num_hex_str(n, 0x10000000, str);
    vbe_put_str(str, ctx);
}

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

ui16 pci_dvc_display_info(vbe_txt_ctx* ctx) {
    ui16 bus, c, inf;
    ui8  dvc;
    pci_dvc_data d;
    char buff[16];
    for (bus = 0; bus < 256; ++bus) {
    for (dvc = 0; dvc < 32;  ++dvc) {
        inf = pci_rd16(bus, dvc, 0, 0);
        if (inf == 0xFFFF) continue;
        if (!pci_query_dvc_info(bus, dvc, &d)) {
            vbe_put_str("Impossible error!  ", ctx);
            continue;
        }
        vbe_put_str("Port    :  ", ctx);
        vbe_put_hex16(bus, ctx);
        vbe_go_next_line_rewind(ctx,0);
        vbe_put_str("Device  :  ", ctx);
        vbe_put_hex16(dvc, ctx);
        vbe_go_next_line_rewind(ctx,0);
        vbe_put_str("VendorID:  ", ctx);
        vbe_put_hex16(d.vendor_id, ctx);
        vbe_go_next_line_rewind(ctx,0);
        vbe_put_str("DeviceID:  ", ctx);
        vbe_put_hex16(d.dvc_id, ctx);
        vbe_go_next_line_rewind(ctx,0);
        vbe_put_str("Class | Subclass: ", ctx);
        vbe_put_hex16(d.cls, ctx);
        vbe_put_str(" | ", ctx);
        vbe_put_hex16(d.subcls,ctx);
        vbe_go_next_line_rewind(ctx, 0);
        vbe_put_str("Header Type", ctx);
        vbe_put_hex16(d.hdr_type,ctx);
        vbe_go_next_line_rewind(ctx, 0);
        vbe_put_str("-----------------", ctx);
        vbe_go_next_line_rewind(ctx,0);
    }}
    return c;
}

#define MEMORY_COUNT ((ui16*)0x8A00)
#define MEMORY_INFO  ((ui8*)0x8A02)
void memory_display(vbe_txt_ctx* ctx) {
    ui32 base_addr; 
    ui32 length;
    ui32 type;
    ui8*  loc;
    char num[16];
    ui16 count = *MEMORY_COUNT;
    cnv_num_hex_str(count, 0x1000, num);
    vbe_put_str("Memory Count: ", ctx);
    vbe_put_str(num, ctx);
    vbe_go_next_line_rewind(ctx, 0);
    for (int i = 0; i < count; ++i) {
        loc       = (MEMORY_INFO+i*24);
        //low base
        base_addr = *(ui32*)loc;
        cnv_num_hex_str(base_addr, 0x10000000, num);
        vbe_put_str(num, ctx);
        vbe_go_next_line_rewind(ctx, 0);
        
        //high base
        base_addr = *(ui32*)(loc + 0x04);
        cnv_num_hex_str(base_addr, 0x10000000, num);
        vbe_put_str(num, ctx);
        vbe_go_next_line_rewind(ctx, 0);

        //length low
        length = *(ui32*)(loc + 0x8);
        cnv_num_hex_str(length, 0x10000000, num);
        vbe_put_str(num, ctx);
        vbe_go_next_line_rewind(ctx, 0);
        
        //length high
        length = *(ui32*)(loc + 0xC);
        cnv_num_hex_str(length, 0x10000000, num);
        vbe_put_str(num, ctx);
        vbe_go_next_line_rewind(ctx, 0);

        //type 
        length = *(ui32*)(loc + 0x10);
        cnv_num_hex_str(length, 0x1000, num);
        vbe_put_str(num, ctx);
        vbe_go_next_line_rewind(ctx, 0);
        vbe_put_str("--------------", ctx);
        vbe_go_next_line_rewind(ctx, 0);
    }
}
