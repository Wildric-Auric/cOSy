#include "ide.h"
#include "pci.h"
#include "port.h"
#include "video.h"


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

ui8 ide_polling_no_check(ide_bus* bus) {
    while (ide_rd_data(bus, ata_data_reg_status_ro) & ata_status_reg_bsy){};
    return 0;
}

ui8 ide_polling(ide_bus* bus) {
    ui8 stat_flag = 0;
    ide_wait400ns(bus);
    //wait bsy to be 0
    while ((stat_flag = ide_rd_data(bus, ata_data_reg_status_ro)) & ata_status_reg_bsy){};
    if (stat_flag & ata_status_reg_err)
        return ata_status_reg_err;
    if (stat_flag & ata_status_reg_df) 
        return ata_status_reg_df;
    if (!(stat_flag & ata_status_reg_drq)) 
        return 0xFF; 
    return  0;
}

//      Drive / Head Register (I/O base + 6)
//      Bit	Abbreviation	Function
//      0 - 3		In CHS addressing, bits 0 to 3 of the head. In LBA addressing, bits 24 to 27 of the block number.
//      4	DRV	Selects the drive number.
//      5	1	Always set.
//      6	LBA	Uses CHS addressing if clear or LBA addressing if set.
//      7	1	Always set.


void vbe_put_hex32(ui32 n, vbe_txt_ctx* ctx);
ui8 ide_init_dvc(ide_dvc* dvc, ide_bus* bus,ui8 drive) {
    ui16 buff[256];
    ui16 wrd;
    ui8  status;
    ui32 w1;
    ui32 w2;
    dvc->bus     = bus;
    dvc->drive   = drive;
    dvc->type    = ide_dvc_type_none;
    dvc->name[0] = 0;
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
    dvc->sign     = buff[0];
    dvc->cap      = buff[ATA_IDENTIFY_CAPABILITIES/2];
    dvc->cmds     = *((ui32*)(buff + ATA_IDENTIFY_COMMAND_SETS/2));
    // Size in sectors; either LBA48 or LBA28 
    w1            = dvc->cmds & (1 << ATA_IDENTIFY_CAPABILITIES_BIG_LBA_BIT);
    w2            = w1        ? ATA_IDENTIFY_MAX_LBA_EXT/2 : ATA_IDENTIFY_MAX_LBA/2;
    dvc->size     = *((ui32*)(buff + w2));
    return 1;
};

//init bus and drive attribute of dvcs
void ide_init_dvcs(ide_dvcs* dvcs, ide_buses* buses) {
    for (int i = 0; i < 4; ++i) {
        ide_init_dvc(&dvcs->lst[i],&buses->lst[i/2] ,i%2);
    }
}

//Assume you have a sectorcount byte and a 28 bit LBA value. A sectorcount of 0 means 256 sectors = 128K.
//
//Notes: When you send a command byte and the RDY bit of the Status Registers is clear, you may have to wait (technically up to 30 seconds) for the drive to spin up, before DRQ sets. You may also need to ignore ERR and DF the first four times that you read the Status, if you are polling.
//
//An example of a 28 bit LBA PIO mode read on the Primary bus:
//
//Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: 
//outb(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
//Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): 
//outb(0x1F1, 0x00)
//Send the sectorcount to port 0x1F2: outb(0x1F2, (unsigned char) count)
//Send the low 8 bits of the LBA to port 0x1F3: outb(0x1F3, (unsigned char) LBA))
//Send the next 8 bits of the LBA to port 0x1F4: outb(0x1F4, (unsigned char)(LBA >> 8))
//Send the next 8 bits of the LBA to port 0x1F5: outb(0x1F5, (unsigned char)(LBA >> 16))
//Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
//Wait for an IRQ or poll.
//Transfer 256 16-bit values, a uint16_t at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
//Then loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
//Note for polling PIO drivers: After transferring the last uint16_t of a PIO data block to the data IO port, give the drive a 400ns delay to reset its DRQ bit (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
//Note on the "magic bits" sent to port 0x1f6: Bit 6 (value = 0x40) is the LBA bit. This must be set for either LBA28 or LBA48 transfers. It must be clear for CHS transfers. Bits 7 and 5 are obsolete for current ATA drives, but must be set for backwards compatibility with very old (ATA1) drives.

bool ide_access_disk_init(ide_dvc* dvc, ui32 lba, ui8 sec_count, ata_access rw) {
    ui8 high_lba = (lba >> 24) & 0x0F; 
    ui8 val      = 0xE0 | (dvc->drive << 4) | high_lba;
    ide_wrt_data(dvc->bus, ata_data_reg_head_reg_rw, val);
    ide_wrt_data(dvc->bus, 0x0, 0x0);
    ide_wrt_data(dvc->bus, ata_data_reg_feat_wo, 0x0);
    ide_wrt_data(dvc->bus, ata_data_reg_sec_count_rw, sec_count); 
    ide_wrt_data(dvc->bus, ata_data_reg_sec_num_rw,     lba & 0xFF);
    ide_wrt_data(dvc->bus, ata_data_reg_cyldr_low_rw,  (lba >> 8)  & 0xFF);
    ide_wrt_data(dvc->bus, ata_data_reg_cyldr_high_rw, (lba >> 16) & 0xFF);
    ide_wrt_data(dvc->bus, ata_data_reg_cmd_wo, rw);
    if (ide_polling(dvc->bus)) 
        return 1;
    return 0;
}

bool ide_rd_disk(ide_dvc* dvc, ui32 lba, ui8 sec_count, ui16* buff) {
    int i,j;
    int cur = 0;
    ide_access_disk_init(dvc, lba, sec_count, ata_access_read);
    //ready to read; todo: add rep insw
    for (i = 0; i < sec_count; ++i) {
        for (j = 0; j < 256; ++j) {
            buff[cur++] = p_in16(dvc->bus->base_io); 
        }
        ide_wait400ns(dvc->bus);
        if (ide_polling(dvc->bus)) 
            return 0;
    }
    return 1;
}

bool ide_wrt_disk(ide_dvc* dvc, ui32 lba, ui8 sec_count, ui16* buff) {
    int i,j;
    int cur = 0;
    ide_access_disk_init(dvc, lba, sec_count, ata_access_write);
    for (i = 0; i < sec_count; ++i) {
        for (j = 0; j < 256; ++j) {
            p_out16(dvc->bus->base_io, buff[cur++]);
            __asm__ volatile("nop\nnop\nnop");
        }
        ide_wait400ns(dvc->bus);
        if (ide_polling(dvc->bus)) 
            return 0;
    }
    ide_wrt_data(dvc->bus, ata_data_reg_cmd_wo, 0xE7); //flush
    ide_polling_no_check(dvc->bus);
    return 1;
}
