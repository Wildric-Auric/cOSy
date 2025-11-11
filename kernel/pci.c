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
    return ((ui32)(pci_rd16(bus,dvc,func,reg_off)) << 16) 
         | ((ui32)(pci_rd16(bus,dvc,func,reg_off + 0x2)));
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
