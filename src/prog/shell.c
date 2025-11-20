#include "util.h"
#include "kb.h"
#include "video.h"

void vbe_display_info(vbe_txt_ctx* ctx);
ui16 pci_dvc_display_info(vbe_txt_ctx* ctx);
void memory_display(vbe_txt_ctx* ctx);
void snake();

static char cmdb[512]; 
static int  cmdp;

static void pchar(char c) {
    cmdb[cmdp++] = c;
    cmdb[cmdp]   = 0;
}

static void rchar() {
   cmdb[cmdp = mmax(cmdp-1,0)] = 0;
}

void shell_help(vbe_txt_ctx* ctx) {
    vbe_put_str("Some Available Commands:", ctx);
    vbe_go_next_line_rewind(ctx,0);
    vbe_put_str("   clear   : clears the buffer", ctx);
    vbe_go_next_line_rewind(ctx,0);
    vbe_put_str("   vbe_info: display vesa mode info", ctx);
    vbe_go_next_line_rewind(ctx,0);
    vbe_put_str("   pci_info: display pci devices info", ctx);
    vbe_go_next_line_rewind(ctx,0);
    vbe_put_str("   mem_info: display physical memory info", ctx);
    vbe_go_next_line_rewind(ctx,0);
}

static char buff[64]; 
void shell_exec(vbe_txt_ctx* ctx) {
    char c;
    int i   = 0;
    int j   = -1;
    buff[0] = 0;
    while ((c = cmdb[++j]) == ' ');
    while ((c = cmdb[j++]) && c != ' ') {
        buff[i++] = c;
        buff[i]   = 0;
    }
    if (str_eq(buff, "clear")) {
        vbe_clear(ctx);
        vbe_go_next_line_rewind(ctx,0);
        vbe_clear(ctx);
    }
    else if (str_eq(buff, "vbe_info")) {
        f2 p = ctx->size;
        ctx->size.x = 1.0; ctx->size.y = 1.0;
        vbe_display_info(ctx);
        vbe_go_next_line_rewind(ctx, 0);
        ctx->size = p;
    }
    else if (str_eq(buff, "pci_info")) {
        f2 p = ctx->size;
        ctx->size.x = 1.0; ctx->size.y = 1.0;
        pci_dvc_display_info(ctx);
        vbe_go_next_line_rewind(ctx, 0);
        ctx->size = p;
    }
    else if (str_eq(buff, "mem_info")) {
        f2 p = ctx->size;
        ctx->size.x = 1.0; ctx->size.y = 1.0;
        memory_display(ctx);
        vbe_go_next_line_rewind(ctx, 0);
        ctx->size = p;
    }
    else if (str_eq(buff, "help")) {
        shell_help(ctx);
        vbe_go_next_line_rewind(ctx, 0);
    }
    else if (str_eq(buff, "snake")) {
        snake();  
        ctx->cur.y = vbe_get_mode_info()->height - 1;
        ctx->cur.x = vbe_get_mode_info()->width  - 1;
        vbe_clear(ctx);
    }
    else {
        vbe_put_str("Unkown Command", ctx);
        vbe_go_next_line_rewind(ctx, 0);
    }

    cmdp    = 0;
    cmdb[0] = 0;
}

void term() {
    vbe_txt_ctx ctx;
    bool shft_mode = 0;
    vbe_init_ctx_def(&ctx);
    ctx.size.x = 2;
    ctx.size.y = 2;
    cmdb[0] = 0;
    cmdp    = 0;
    while (1) {
    while (kb_get_stack_ptr()) {
        ui16    sc   = kb_pop_scode();
        key_inf ki;
        kb_cnv_scode(sc, &ki);
        bool prs     = ki.event & key_event_enm_pressed;
        bool isan    = is_alpha_num(ki.key);
        if (ki.key == LShift_Key) {
            shft_mode = prs; 
        }
        else if (shft_mode && isan) {
            if (is_alpha(ki.key)) {
                ki.key -= 20;
            }
            else {
                ki.key = ki.key == '8' ? '_' : ki.key;
            }
            vbe_put_char(ki.key, &ctx);
            vbe_render_gcur(&ctx);
            pchar(ki.key);
        }
        else if (prs && isan) {
            vbe_put_char(ki.key, &ctx);
            vbe_render_gcur(&ctx);
            pchar(ki.key);
        }
        else if (ki.key == Enter_Key && prs) {
            vbe_put_char(' ', &ctx);
            vbe_go_last_char(&ctx);
            vbe_go_next_line_rewind(&ctx, 0);
            shell_exec(&ctx);
            vbe_render_gcur(&ctx);
        }
        else if (ki.key == Space_Key && prs) {
            vbe_put_char(' ',&ctx);
            vbe_render_gcur(&ctx);
            pchar(ki.key);
        }
        else if (ki.key == Back_Key  && prs) {
            vbe_put_char(' ', &ctx);
            vbe_go_last_char(&ctx);
            vbe_clear_char(&ctx);
            vbe_render_gcur(&ctx);
            rchar();
        }
    }
    }
}
