#include "globals.h"
#include "pci.h"

//each cable is linked to 4 drives: 11 primary master, 12 primary slave, 21 secondary master, 22 secondary slave
//each bus (primary, secondary) has ports

//must be divided by 2 to get the real register 
//done because 2 
enum ata_data_reg_enum {
    ata_data_reg_data_rw            = 0,  //0
    ata_data_reg_err_ro             = 2,  //1
    ata_data_reg_feat_wo            = 4,  //2
    ata_data_reg_sec_count_rw       = 6,  //3
    ata_data_reg_sec_num_rw         = 8,  //4
    ata_data_reg_cyldr_low_rw       = 10, //5
    ata_data_reg_cyldr_high_rw      = 12, //6
    ata_data_reg_status_wo          = 14, //7
    ata_data_reg_cmd_ro             = 15, //8
};

enum ata_ctrl_reg_enum {
    ata_ctrl_reg_alt_status   = 0,
    ata_ctrl_reg_dvc_ctrl     = 1,
    ata_ctrl_reg_drv_addr     = 2
};

typedef union {
    struct {
        ui32 bar0;
        ui32 bar1;
        ui32 bar2;
        ui32 bar3;
        ui32 bar4;
    };
    struct {
        ui32 base_io_1;
        ui32 base_ctrl_1;
        ui32 base_io_2;
        ui32 base_ctrl_2;
        ui32 base_master;
    };
} ide_base_addrs;

typedef struct {
    ui16 base_io; 
    ui16 base_ctrl;
    ui16 master;
    ui8  no_int;
} ide_bus;

typedef struct {
    ide_bus primary;
    ide_bus secondary;
} ide_buses;

typedef struct {
    ui8 reserved;
    ui8 bus;          //0 primary, 1 secondary
    ui8 drive;        //0 master, 1 slave 
    ui16 type;        //0 ATA, 1 ARAPI
    ui16 sign;        //signature
    ui16 cap;
    ui32 cmds;        //command sets supported
    ui32 size;        //size in sectors
    char name[41];
} ide_dvc;

typedef ide_dvc ide_dvcs[4];

ide_buses ide_ports_container;

void ide_init(ide_base_addrs* arg, ide_buses* buses) {
    const ui32 c = ~0x3; // to clear 2 last bytes
    arg->bar0  &= c; arg->bar1  &= c;
    arg->bar2  &= c; arg->bar3  &= c;
    arg->bar4  &= c;
    buses->primary.base_io     = arg->base_io_1;
    buses->secondary.base_io   = arg->base_io_2;
    buses->primary.base_ctrl   = arg->base_ctrl_1;
    buses->secondary.base_ctrl = arg->base_ctrl_2;
    buses->primary.master      = arg->base_master;
    buses->secondary.master    = arg->base_master + 8;
};

bool ide_init_pci(pci_dvc_loc_info* info, ide_buses* out_buses) {
    ide_base_addrs addrs;
    if (info->data.hdr_type)
       return 0; 
    addrs.bar0 = pci_rd32(info->bus, info->dvc, 0, 16);
    addrs.bar1 = pci_rd32(info->bus, info->dvc, 0, 20);
    addrs.bar2 = pci_rd32(info->bus, info->dvc, 0, 24);
    addrs.bar3 = pci_rd32(info->bus, info->dvc, 0, 28);
    addrs.bar4 = pci_rd32(info->bus, info->dvc, 0, 32);
    ide_init(&addrs, out_buses);
    return 1;
}

