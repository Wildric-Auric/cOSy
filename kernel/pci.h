// https://wiki.osdev.org/PCI
// https://www.youtube.com/watch?v=Y4OVelU1ytk

#ifndef PCI_H
#define PCI_H
#include "globals.h"

#define PCI_CMDP   0xCF8
#define PCI_DATAP  0xCFC
#define _PCI_MAKE_ADDR(bus, dvc, func, reg_off) 0x80000000 | (bus << 16) | (dvc << 11) | (func << 8) | (reg_off & 0xFC)
#define  PCI_MAKE_ADDR(bus, dvc, func, reg_off) _PCI_MAKE_ADDR(((ui32)(bus)),((ui32)(dvc)), ((ui32)(func)),((ui32)(reg_off)))

typedef struct {
    ui16 vendor_id;
    ui16 dvc_id;
    ui8 rev_id;
    ui8 prog_if;
    ui8 cls;
    ui8 subcls;
    ui8 cache_size;
    ui8 ltc_tmr;
    ui8 hdr_type;
    ui8 bist;
} pci_dvc_data;

typedef struct {
    ui16 bus;
    ui8  dvc;
    pci_dvc_data data;
} pci_dvc_loc_info;

ui16    pci_rd16_direct(ui32 addr, ui8 reg_off);
ui16    pci_rd16(ui8 bus, ui8 dvc, ui8 func, ui8 reg_off);
ui32    pci_rd32(ui8 bus, ui8 dvc, ui8 func, ui8 reg_off);
bool    pci_query_dvc_info(ui8 bus, ui8 dvc, pci_dvc_data*);
bool    pci_dvc_find_next_class(pci_dvc_loc_info*);
ui16    pci_dvc_count();

#endif
