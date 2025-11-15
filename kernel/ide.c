#include "ide.h"
#include "pci.h"
#include "port.h"

//offset in bytes
#define ATA_IDENTIFY_MODEL_OFFSET 54

void ide_init(ide_base_addrs* arg, ide_buses* buses) {
    buses->primary.idx         = 0;
    buses->secondary.idx       = 1;
    buses->primary.base_io     = arg->base_io_1;
    buses->secondary.base_io   = arg->base_io_2;
    buses->primary.base_ctrl   = arg->base_ctrl_1;
    buses->secondary.base_ctrl = arg->base_ctrl_2;
    buses->primary.master      = arg->base_master;
    buses->secondary.master    = arg->base_master + 8;
};

void ide_base_fix_addr(ide_base_addrs* addrs, bool comp) {
    const ui32 c = ~0x3; // to clear 2 last bytes
    if (comp) {
        addrs->bar0 = 0x1F0;
        addrs->bar1 = 0x3F6;
        addrs->bar2 = 0x170;
        addrs->bar3 = 0x376;
        addrs->bar4 = 0xC000;
        return;
    }
    addrs->bar0  = (addrs->bar0 & c); addrs->bar1  = (addrs->bar1 & c);
    addrs->bar2  = (addrs->bar2 & c); addrs->bar3  = (addrs->bar3 & c);
    addrs->bar4  = (addrs->bar4 & c);
}

bool ide_init_pci(pci_dvc_loc_info* info, ide_buses* out_buses) {
    ide_base_addrs addrs;
    if (info->data.hdr_type)
       return 0; 
    addrs.bar0 = pci_rd32(info->bus, info->dvc, 0, 16);
    addrs.bar1 = pci_rd32(info->bus, info->dvc, 0, 20);
    addrs.bar2 = pci_rd32(info->bus, info->dvc, 0, 24);
    addrs.bar3 = pci_rd32(info->bus, info->dvc, 0, 28);
    addrs.bar4 = pci_rd32(info->bus, info->dvc, 0, 32);
    ide_base_fix_addr(&addrs, !info->data.prog_if); //todo:: check if we only are in compatibility mode 
    ide_init(&addrs, out_buses);
    return 1;
}

bool ide_init_legacy(ide_buses* out_buses) {
    ide_base_addrs addrs = {};
    ide_base_fix_addr(&addrs, 1);
    ide_init(&addrs, out_buses);
    return 1;   
}

void ide_wrt_data(ide_bus* bus, ata_data_reg_enum reg, ui8 data) {
    reg /= 2;
    p_out(bus->base_io + reg, data);
}
ui8 ide_rd_data(ide_bus* bus, ata_data_reg_enum reg) {
    reg /= 2;
    ui8 res;
    res = p_in(bus->base_io + reg);
    return res;
}
void ide_wrt_ctrl(ide_bus* bus, ata_ctrl_reg_enum reg, ui8 data) {
    reg /= 2;
    p_out(bus->base_ctrl + reg, data);
}
ui8 ide_rd_ctrl(ide_bus* bus, ata_ctrl_reg_enum reg) {
    reg /= 2;
    ui8 res;
    res = p_in(bus->base_ctrl + reg);
    return res;
}

void ide_disable_irq(ide_bus* bus) {
    ide_wrt_ctrl(bus, ata_ctrl_reg_dvc_ctrl_wo, 0x2);
}

void ide_wait400ns(ide_bus* bus) {
    for (int i = 0; i < 15;++i)
        ide_rd_ctrl(bus, ata_ctrl_reg_alt_status_ro);
}

ui8 ide_polling(ide_bus* bus) {
    ui8 err = 0;
    ide_wait400ns(bus);
    //wait bsy to be 0
    while (1) {
        ui8 stat_flag = ide_rd_data(bus, ata_data_reg_status_ro);
        ui8 bsy       = !(stat_flag & ata_status_reg_bsy); // clear to end
        ui8 drq       = stat_flag & ata_status_reg_drq; 
        ui8 cond1     = bsy && drq;
        if (stat_flag & ata_status_reg_err) {
            err = ata_status_reg_err;
            break;
        }
        else if (stat_flag & ata_status_reg_df) {
            err = ata_status_reg_df;
            break;
        }
        if (drq)
            break;
    }
    return err;
}

//      Drive / Head Register (I/O base + 6)
//      Bit	Abbreviation	Function
//      0 - 3		In CHS addressing, bits 0 to 3 of the head. In LBA addressing, bits 24 to 27 of the block number.
//      4	DRV	Selects the drive number.
//      5	1	Always set.
//      6	LBA	Uses CHS addressing if clear or LBA addressing if set.
//      7	1	Always set.

ui8 ide_init_dvc(ide_dvc* dvc, ide_bus* bus,ui8 drive) {
    ui16 buff[256];
    ui16 wrd;
    ui8  status;
    dvc->bus     = bus;
    dvc->drive   = drive;
    dvc->type    = 0;
    //dvc->name[0] = 0;
    //0b10110000 -> selects drive 1 of bus  in CHS
    ide_wrt_data(bus, ata_data_reg_head_reg_rw, 0xA0 | (dvc->drive << 4));
    ide_wait400ns(bus);
    status = ide_rd_data(bus, ata_data_reg_status_ro);
    if (!status || status == 0xFF) return 0;

    ide_wrt_data(bus, ata_data_reg_sec_count_rw,0);
    ide_wrt_data(bus, ata_data_reg_sec_num_rw,0);
    ide_wrt_data(bus, ata_data_reg_cyldr_low_rw,0);
    ide_wrt_data(bus, ata_data_reg_cyldr_high_rw,0);
    ide_wrt_data(bus, ata_data_reg_cmd_wo, 0xEC);
 
    ide_wait400ns(bus);
    status = ide_rd_data(bus, ata_data_reg_status_ro);
    if (!status || status == 0xFF) 
        return 0;
    ui32 hcyl; 
    ui32 lcyl;

    //ide_wait400ns(bus);
    while ((status = ide_rd_data(bus, ata_data_reg_status_ro)) & ata_status_reg_bsy) {}    
    //if (ide_polling(bus)) return 0;
    hcyl = ide_rd_data(bus, ata_data_reg_cyldr_high_rw);
    lcyl = ide_rd_data(bus, ata_data_reg_cyldr_low_rw);

    if (!hcyl && !lcyl) {
        while (1) {
            status = ide_rd_data(bus, ata_data_reg_status_ro);
            if (status & ata_status_reg_drq)
                break;
            if (status & ata_status_reg_err) 
                return 0;
        }
    }

    if (lcyl == 0x14 && hcyl == 0xEB) dvc->type = ide_dvc_type_patapi;
    if (lcyl == 0x69 && hcyl == 0x96) dvc->type = ide_dvc_type_satapi;
    if (lcyl == 0x00 && hcyl == 0x00) dvc->type = ide_dvc_type_pata;
    if (lcyl == 0x3C && hcyl == 0xC3) dvc->type = ide_dvc_type_sata;

    for (int i = 0; i < 256; ++i) {
        buff[i] = p_in16(bus->base_io);
    }
    for (int i = 0; i < 40; i += 2) {
        wrd = buff[ATA_IDENTIFY_MODEL_OFFSET/2 + i/2];
        dvc->name[i]   = (wrd >> 8) & 0xFF;
        dvc->name[i+1] = wrd & 0xFF;
    }
    dvc->name[40] = 0;

    return 1;
};

//init bus and drive attribute of dvcs
void ide_init_dvcs(ide_dvcs* dvcs, ide_buses* buses) {
    for (int i = 0; i < 4; ++i) {
        ide_init_dvc(&dvcs->lst[i],&buses->lst[i/2] ,i%2);
    }
}

