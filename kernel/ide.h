#ifndef IDE_H
#define IDE_H

#include "globals.h"
#include "pci.h"


//each cable is linked to 4 drives: 11 primary master, 12 primary slave, 21 secondary master, 22 secondary slave
//each bus (primary, secondary) has ports
//must be divided by 2 to get the real register 
//done because 2 
typedef enum  {
    ata_data_reg_data_rw            = 0,  //0
    ata_data_reg_err_ro             = 1,  //1
    ata_data_reg_feat_wo            = 2,  //1
    ata_data_reg_sec_count_rw       = 4,  //2
    ata_data_reg_sec_num_rw         = 6,  //3  LBAIo
    ata_data_reg_cyldr_low_rw       = 8,  //4  LBAmid
    ata_data_reg_cyldr_high_rw      = 10, //5  LBAhigh
    ata_data_reg_head_reg_rw        = 12, //6   
    ata_data_reg_status_ro          = 14, //7
    ata_data_reg_cmd_wo             = 15, //7
} ata_data_reg_enum;

typedef enum {
    ata_ctrl_reg_alt_status_ro   = 0, //0
    ata_ctrl_reg_dvc_ctrl_wo     = 1, //0
    ata_ctrl_reg_drv_addr_ro     = 2  //1
} ata_ctrl_reg_enum;

typedef enum {
    ata_status_reg_err     = 1,
    ata_status_reg_idx     = 1<<1,
    ata_status_reg_corr    = 1<<2,
    ata_status_reg_drq     = 1<<3,
    ata_status_reg_srv     = 1<<4,
    ata_status_reg_df      = 1<<5,
    ata_status_reg_rdy     = 1<<6,
    ata_status_reg_bsy     = 1<<7,
} ata_status_reg_enum;

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
    ui8  idx;
    ui32 base_io; 
    ui32 base_ctrl;
    ui32 master;
    ui8  no_int;
} ide_bus;

typedef union {
    struct {
        ide_bus primary;
        ide_bus secondary;
    };
    ide_bus lst[2];
} ide_buses;

typedef enum {
    ide_dvc_type_none,
    ide_dvc_type_patapi,
    ide_dvc_type_satapi,
    ide_dvc_type_pata,
    ide_dvc_type_sata,
} ide_dvc_type;

typedef struct {
    ide_bus*      bus;          //0 primary, 1 secondary
    ide_dvc_type type;        //ATA, ATAPI ...
    ui8          drive;       //0 master, 1 slave 
    ui16         sign;        //signature
    ui16         cap; 
    ui32         cmds;        //command sets supported
    ui32         size;        //size in sectors
    char         name[41];
} ide_dvc;

typedef struct {
    ide_dvc lst[4];
} ide_dvcs;
void ide_init(ide_base_addrs* arg, ide_buses* buses);
bool ide_init_pci(pci_dvc_loc_info* info, ide_buses* out_buses);
bool ide_init_legacy(ide_buses* out_buses);
void ide_wrt_data(ide_bus* bus, ata_data_reg_enum reg, ui8 data);
ui8 ide_rd_data(ide_bus* bus, ata_data_reg_enum reg);
void ide_wrt_ctrl(ide_bus* bus, ata_ctrl_reg_enum reg, ui8 data);
ui8 ide_rd_ctrl(ide_bus* bus, ata_ctrl_reg_enum reg);
void ide_disable_irq(ide_bus* bus);
void ide_wait400ns(ide_bus* bus);
ui8 ide_polling(ide_bus* bus);
ui8 ide_select_dvc(ide_dvc* dvc);
void ide_init_dvcs(ide_dvcs* dvcs, ide_buses* buses);

#endif
