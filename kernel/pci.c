#include "pci.h"
#include "port.h"
#include "video.h"
#include "util.h"

#define PCI_CMDP   0xCF8
#define PCI_DATAP  0xCFC


ui16 pci_rd16_direct(ui32 addr, ui8 reg_off) {
   p_out32(PCI_CMDP, addr); 
   ui32 res = p_in32(PCI_DATAP);
   res =  (res >> ((reg_off & 0x2) << 0x3)) & 0xFFFF;
   return res;
}

ui16 pci_rd16(ui8 bus, ui8 dvc, ui8 func, ui8 reg_off) {
    return pci_rd16_direct(PCI_MAKE_ADDR(bus, dvc, func, reg_off), reg_off);
}

ui32 pci_rd32(ui8 bus, ui8 dvc, ui8 func, ui8 reg_off) {
    ui32 res;
    return ((ui32)(pci_rd16(bus,dvc,func,reg_off))) 
         | ((ui32)(pci_rd16(bus,dvc,func,reg_off + 0x2)) << 16);
}

bool pci_dvc_find_next_class(pci_dvc_loc_info* info) {
    ui8  dvc;
    ui16 bus;
    ui16 val;
    for (bus = info->bus; bus < 256; ++bus) {
    for (dvc = info->dvc; dvc < 32;  ++dvc) {
        if (pci_rd16(bus, dvc, 0, 0) == 0xFFFF) 
            continue;
        val = pci_rd16(bus, dvc, 0x0, 0xA);
        if (((val & 0xFF) != info->data.subcls) | ( ((val>>8) & 0xFF) != info->data.cls))  //todo::check order
            continue; 
        pci_query_dvc_info(bus, 0, &info->data);
        info->bus = bus;
        info->dvc = dvc;
        return 1;
    }}
    return 0;
}

bool pci_query_dvc_info(ui8 bus, ui8 dvc, pci_dvc_data* d) {
#define pci_q(off) val = pci_rd16(bus, dvc, 0, off)
    ui16 val;
    pci_q(0);
    if (val == 0xFFFF)
        return 0;
    d->vendor_id = val;
    pci_q(2);
    d->dvc_id    = val;
    pci_q(8);
    d->rev_id    = (val   )&0xFF;
    d->prog_if   = (val>>8)&0xFF;
    pci_q(10);
    d->subcls    = val&0xFF;
    d->cls       = (val>>8)&0xFF;
    pci_q(12);
    d->cache_size= val&0xFF;
    d->ltc_tmr   = (val>>8)&0xFF;
    pci_q(14);
    d->hdr_type  = val&0xFF;
    d->bist      = (val>>8)&0xFF;
    return 1;
#undef pci_q
}

ui16 pci_dvc_count() {
    ui8  dvc;
    ui16 bus;
    ui16 c = 0;
    for (bus = 0; bus < 256; ++bus) {
    for (dvc = 0; dvc < 32;  ++dvc) {
        c +=  (pci_rd16(bus, dvc, 0, 0) != 0xFFFF);
    }}
    return c;
}
