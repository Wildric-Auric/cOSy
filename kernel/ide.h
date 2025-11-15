#ifndef IDE_H
#define IDE_H

#include "globals.h"
#include "pci.h"

// https://wiki.osdev.org/ATA_PIO_Mode
// https://wiki.osdev.org/PCI_IDE_Controller

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
    ide_dvc_type_none       = 0,
    ide_dvc_type_patapi     = 1,
    ide_dvc_type_satapi     = 2,
    ide_dvc_type_pata       = 3,
    ide_dvc_type_sata       = 4,
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



//IDENTIFY 0xEC command makes IO data readable, the following struct should be read
//source: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ata/ns-ata-_identify_device_data?redirectedfrom=MSDN

//typedef struct _IDENTIFY_DEVICE_DATA {
//  struct {
//    ui16 Reserved1 : 1;
//    ui16 Retired3 : 1;
//    ui16 ResponseIncomplete : 1;
//    ui16 Retired2 : 3;
//    ui16 FixedDevice : 1;
//    ui16 RemovableMedia : 1;
//    ui16 Retired1 : 7;
//    ui16 DeviceType : 1;
//  } GeneralConfiguration;
//  ui16 NumCylinders;
//  ui16 SpecificConfiguration;
//  ui16 NumHeads;
//  ui16 Retired1[2];
//  ui16 NumSectorsPerTrack;
//  ui16 VendorUnique1[3];
//  ui8  SerialNumber[20];
//  ui16 Retired2[2];
//  ui16 Obsolete1;
//  ui8  FirmwareRevision[8];
//  ui8  ModelNumber[40];
//  ui8  MaximumBlockTransfer;
//  ui8  VendorUnique2;
//  struct {
//    ui16 FeatureSupported : 1;
//    ui16 Reserved : 15;
//  } TrustedComputing;
//  struct {
//    ui8  CurrentLongPhysicalSectorAlignment : 2;
//    ui8  ReservedByte49 : 6;
//    ui8  DmaSupported : 1;
//    ui8  LbaSupported : 1;
//    ui8  IordyDisable : 1;
//    ui8  IordySupported : 1;
//    ui8  Reserved1 : 1;
//    ui8  StandybyTimerSupport : 1;
//    ui8  Reserved2 : 2;
//    ui16 ReservedWord50;
//  } Capabilities;
//  ui16 ObsoleteWords51[2];
//  ui16 TranslationFieldsValid : 3;
//  ui16 Reserved3 : 5;
//  ui16 FreeFallControlSensitivity : 8;
//  ui16 NumberOfCurrentCylinders;
//  ui16 NumberOfCurrentHeads;
//  ui16 CurrentSectorsPerTrack;
//  ui32  CurrentSectorCapacity;
//  ui8  CurrentMultiSectorSetting;
//  ui8  MultiSectorSettingValid : 1;
//  ui8  ReservedByte59 : 3;
//  ui8  SanitizeFeatureSupported : 1;
//  ui8  CryptoScrambleExtCommandSupported : 1;
//  ui8  OverwriteExtCommandSupported : 1;
//  ui8  BlockEraseExtCommandSupported : 1;
//  ui32  UserAddressableSectors;
//  ui16 ObsoleteWord62;
//  ui16 MultiWordDMASupport : 8;
//  ui16 MultiWordDMAActive : 8;
//  ui16 AdvancedPIOModes : 8;
//  ui16 ReservedByte64 : 8;
//  ui16 MinimumMWXferCycleTime;
//  ui16 RecommendedMWXferCycleTime;
//  ui16 MinimumPIOCycleTime;
//  ui16 MinimumPIOCycleTimeIORDY;
//  struct {
//    ui16 ZonedCapabilities : 2;
//    ui16 NonVolatileWriteCache : 1;
//    ui16 ExtendedUserAddressableSectorsSupported : 1;
//    ui16 DeviceEncryptsAllUserData : 1;
//    ui16 ReadZeroAfterTrimSupported : 1;
//    ui16 Optional28BitCommandsSupported : 1;
//    ui16 IEEE1667 : 1;
//    ui16 DownloadMicrocodeDmaSupported : 1;
//    ui16 SetMaxSetPasswordUnlockDmaSupported : 1;
//    ui16 WriteBufferDmaSupported : 1;
//    ui16 ReadBufferDmaSupported : 1;
//    ui16 DeviceConfigIdentifySetDmaSupported : 1;
//    ui16 LPSAERCSupported : 1;
//    ui16 DeterministicReadAfterTrimSupported : 1;
//    ui16 CFastSpecSupported : 1;
//  } AdditionalSupported;
//  ui16 ReservedWords70[5];
//  ui16 QueueDepth : 5;
//  ui16 ReservedWord75 : 11;
//  struct {
//    ui16 Reserved0 : 1;
//    ui16 SataGen1 : 1;
//    ui16 SataGen2 : 1;
//    ui16 SataGen3 : 1;
//    ui16 Reserved1 : 4;
//    ui16 NCQ : 1;
//    ui16 HIPM : 1;
//    ui16 PhyEvents : 1;
//    ui16 NcqUnload : 1;
//    ui16 NcqPriority : 1;
//    ui16 HostAutoPS : 1;
//    ui16 DeviceAutoPS : 1;
//    ui16 ReadLogDMA : 1;
//    ui16 Reserved2 : 1;
//    ui16 CurrentSpeed : 3;
//    ui16 NcqStreaming : 1;
//    ui16 NcqQueueMgmt : 1;
//    ui16 NcqReceiveSend : 1;
//    ui16 DEVSLPtoReducedPwrState : 1;
//    ui16 Reserved3 : 8;
//  } SerialAtaCapabilities;
//  struct {
//    ui16 Reserved0 : 1;
//    ui16 NonZeroOffsets : 1;
//    ui16 DmaSetupAutoActivate : 1;
//    ui16 DIPM : 1;
//    ui16 InOrderData : 1;
//    ui16 HardwareFeatureControl : 1;
//    ui16 SoftwareSettingsPreservation : 1;
//    ui16 NCQAutosense : 1;
//    ui16 DEVSLP : 1;
//    ui16 HybridInformation : 1;
//    ui16 Reserved1 : 6;
//  } SerialAtaFeaturesSupported;
//  struct {
//    ui16 Reserved0 : 1;
//    ui16 NonZeroOffsets : 1;
//    ui16 DmaSetupAutoActivate : 1;
//    ui16 DIPM : 1;
//    ui16 InOrderData : 1;
//    ui16 HardwareFeatureControl : 1;
//    ui16 SoftwareSettingsPreservation : 1;
//    ui16 DeviceAutoPS : 1;
//    ui16 DEVSLP : 1;
//    ui16 HybridInformation : 1;
//    ui16 Reserved1 : 6;
//  } SerialAtaFeaturesEnabled;
//  ui16 MajorRevision;
//  ui16 MinorRevision;
//  struct {
//    ui16 SmartCommands : 1;
//    ui16 SecurityMode : 1;
//    ui16 RemovableMediaFeature : 1;
//    ui16 PowerManagement : 1;
//    ui16 Reserved1 : 1;
//    ui16 WriteCache : 1;
//    ui16 LookAhead : 1;
//    ui16 ReleaseInterrupt : 1;
//    ui16 ServiceInterrupt : 1;
//    ui16 DeviceReset : 1;
//    ui16 HostProtectedArea : 1;
//    ui16 Obsolete1 : 1;
//    ui16 WriteBuffer : 1;
//    ui16 ReadBuffer : 1;
//    ui16 Nop : 1;
//    ui16 Obsolete2 : 1;
//    ui16 DownloadMicrocode : 1;
//    ui16 DmaQueued : 1;
//    ui16 Cfa : 1;
//    ui16 AdvancedPm : 1;
//    ui16 Msn : 1;
//    ui16 PowerUpInStandby : 1;
//    ui16 ManualPowerUp : 1;
//    ui16 Reserved2 : 1;
//    ui16 SetMax : 1;
//    ui16 Acoustics : 1;
//    ui16 BigLba : 1;
//    ui16 DeviceConfigOverlay : 1;
//    ui16 FlushCache : 1;
//    ui16 FlushCacheExt : 1;
//    ui16 WordValid83 : 2;
//    ui16 SmartErrorLog : 1;
//    ui16 SmartSelfTest : 1;
//    ui16 MediaSerialNumber : 1;
//    ui16 MediaCardPassThrough : 1;
//    ui16 StreamingFeature : 1;
//    ui16 GpLogging : 1;
//    ui16 WriteFua : 1;
//    ui16 WriteQueuedFua : 1;
//    ui16 WWN64Bit : 1;
//    ui16 URGReadStream : 1;
//    ui16 URGWriteStream : 1;
//    ui16 ReservedForTechReport : 2;
//    ui16 IdleWithUnloadFeature : 1;
//    ui16 WordValid : 2;
//  } CommandSetSupport;
//  struct {
//    ui16 SmartCommands : 1;
//    ui16 SecurityMode : 1;
//    ui16 RemovableMediaFeature : 1;
//    ui16 PowerManagement : 1;
//    ui16 Reserved1 : 1;
//    ui16 WriteCache : 1;
//    ui16 LookAhead : 1;
//    ui16 ReleaseInterrupt : 1;
//    ui16 ServiceInterrupt : 1;
//    ui16 DeviceReset : 1;
//    ui16 HostProtectedArea : 1;
//    ui16 Obsolete1 : 1;
//    ui16 WriteBuffer : 1;
//    ui16 ReadBuffer : 1;
//    ui16 Nop : 1;
//    ui16 Obsolete2 : 1;
//    ui16 DownloadMicrocode : 1;
//    ui16 DmaQueued : 1;
//    ui16 Cfa : 1;
//    ui16 AdvancedPm : 1;
//    ui16 Msn : 1;
//    ui16 PowerUpInStandby : 1;
//    ui16 ManualPowerUp : 1;
//    ui16 Reserved2 : 1;
//    ui16 SetMax : 1;
//    ui16 Acoustics : 1;
//    ui16 BigLba : 1;
//    ui16 DeviceConfigOverlay : 1;
//    ui16 FlushCache : 1;
//    ui16 FlushCacheExt : 1;
//    ui16 Resrved3 : 1;
//    ui16 Words119_120Valid : 1;
//    ui16 SmartErrorLog : 1;
//    ui16 SmartSelfTest : 1;
//    ui16 MediaSerialNumber : 1;
//    ui16 MediaCardPassThrough : 1;
//    ui16 StreamingFeature : 1;
//    ui16 GpLogging : 1;
//    ui16 WriteFua : 1;
//    ui16 WriteQueuedFua : 1;
//    ui16 WWN64Bit : 1;
//    ui16 URGReadStream : 1;
//    ui16 URGWriteStream : 1;
//    ui16 ReservedForTechReport : 2;
//    ui16 IdleWithUnloadFeature : 1;
//    ui16 Reserved4 : 2;
//  } CommandSetActive;
//  ui16 UltraDMASupport : 8;
//  ui16 UltraDMAActive : 8;
//  struct {
//    ui16 TimeRequired : 15;
//    ui16 ExtendedTimeReported : 1;
//  } NormalSecurityEraseUnit;
//  struct {
//    ui16 TimeRequired : 15;
//    ui16 ExtendedTimeReported : 1;
//  } EnhancedSecurityEraseUnit;
//  ui16 CurrentAPMLevel : 8;
//  ui16 ReservedWord91 : 8;
//  ui16 MasterPasswordID;
//  ui16 HardwareResetResult;
//  ui16 CurrentAcousticValue : 8;
//  ui16 RecommendedAcousticValue : 8;
//  ui16 StreamMinRequestSize;
//  ui16 StreamingTransferTimeDMA;
//  ui16 StreamingAccessLatencyDMAPIO;
//  ui32  StreamingPerfGranularity;
//  ui32  Max48BitLBA[2];
//  ui16 StreamingTransferTime;
//  ui16 DsmCap;
//  struct {
//    ui16 LogicalSectorsPerPhysicalSector : 4;
//    ui16 Reserved0 : 8;
//    ui16 LogicalSectorLongerThan256Words : 1;
//    ui16 MultipleLogicalSectorsPerPhysicalSector : 1;
//    ui16 Reserved1 : 2;
//  } PhysicalLogicalSectorSize;
//  ui16 InterSeekDelay;
//  ui16 WorldWideName[4];
//  ui16 ReservedForWorldWideName128[4];
//  ui16 ReservedForTlcTechnicalReport;
//  ui16 WordsPerLogicalSector[2];
//  struct {
//    ui16 ReservedForDrqTechnicalReport : 1;
//    ui16 WriteReadVerify : 1;
//    ui16 WriteUncorrectableExt : 1;
//    ui16 ReadWriteLogDmaExt : 1;
//    ui16 DownloadMicrocodeMode3 : 1;
//    ui16 FreefallControl : 1;
//    ui16 SenseDataReporting : 1;
//    ui16 ExtendedPowerConditions : 1;
//    ui16 Reserved0 : 6;
//    ui16 WordValid : 2;
//  } CommandSetSupportExt;
//  struct {
//    ui16 ReservedForDrqTechnicalReport : 1;
//    ui16 WriteReadVerify : 1;
//    ui16 WriteUncorrectableExt : 1;
//    ui16 ReadWriteLogDmaExt : 1;
//    ui16 DownloadMicrocodeMode3 : 1;
//    ui16 FreefallControl : 1;
//    ui16 SenseDataReporting : 1;
//    ui16 ExtendedPowerConditions : 1;
//    ui16 Reserved0 : 6;
//    ui16 Reserved1 : 2;
//  } CommandSetActiveExt;
//  ui16 ReservedForExpandedSupportandActive[6];
//  ui16 MsnSupport : 2;
//  ui16 ReservedWord127 : 14;
//  struct {
//    ui16 SecuritySupported : 1;
//    ui16 SecurityEnabled : 1;
//    ui16 SecurityLocked : 1;
//    ui16 SecurityFrozen : 1;
//    ui16 SecurityCountExpired : 1;
//    ui16 EnhancedSecurityEraseSupported : 1;
//    ui16 Reserved0 : 2;
//    ui16 SecurityLevel : 1;
//    ui16 Reserved1 : 7;
//  } SecurityStatus;
//  ui16 ReservedWord129[31];
//  struct {
//    ui16 MaximumCurrentInMA : 12;
//    ui16 CfaPowerMode1Disabled : 1;
//    ui16 CfaPowerMode1Required : 1;
//    ui16 Reserved0 : 1;
//    ui16 Word160Supported : 1;
//  } CfaPowerMode1;
//  ui16 ReservedForCfaWord161[7];
//  ui16 NominalFormFactor : 4;
//  ui16 ReservedWord168 : 12;
//  struct {
//    ui16 SupportsTrim : 1;
//    ui16 Reserved0 : 15;
//  } DataSetManagementFeature;
//  ui16 AdditionalProductID[4];
//  ui16 ReservedForCfaWord174[2];
//  ui16 CurrentMediaSerialNumber[30];
//  struct {
//    ui16 Supported : 1;
//    ui16 Reserved0 : 1;
//    ui16 WriteSameSuported : 1;
//    ui16 ErrorRecoveryControlSupported : 1;
//    ui16 FeatureControlSuported : 1;
//    ui16 DataTablesSuported : 1;
//    ui16 Reserved1 : 6;
//    ui16 VendorSpecific : 4;
//  } SCTCommandTransport;
//  ui16 ReservedWord207[2];
//  struct {
//    ui16 AlignmentOfLogicalWithinPhysical : 14;
//    ui16 Word209Supported : 1;
//    ui16 Reserved0 : 1;
//  } BlockAlignment;
//  ui16 WriteReadVerifySectorCountMode3Only[2];
//  ui16 WriteReadVerifySectorCountMode2Only[2];
//  struct {
//    ui16 NVCachePowerModeEnabled : 1;
//    ui16 Reserved0 : 3;
//    ui16 NVCacheFeatureSetEnabled : 1;
//    ui16 Reserved1 : 3;
//    ui16 NVCachePowerModeVersion : 4;
//    ui16 NVCacheFeatureSetVersion : 4;
//  } NVCacheCapabilities;
//  ui16 NVCacheSizeLSW;
//  ui16 NVCacheSizeMSW;
//  ui16 NominalMediaRotationRate;
//  ui16 ReservedWord218;
//  struct {
//    ui8 NVCacheEstimatedTimeToSpinUpInSeconds;
//    ui8 Reserved;
//  } NVCacheOptions;
//  ui16 WriteReadVerifySectorCountMode : 8;
//  ui16 ReservedWord220 : 8;
//  ui16 ReservedWord221;
//  struct {
//    ui16 MajorVersion : 12;
//    ui16 TransportType : 4;
//  } TransportMajorVersion;
//  ui16 TransportMinorVersion;
//  ui16 ReservedWord224[6];
//  ui32 ExtendedNumberOfUserAddressableSectors[2];
//  ui16 MinBlocksPerDownloadMicrocodeMode03;
//  ui16 MaxBlocksPerDownloadMicrocodeMode03;
//  ui16 ReservedWord236[19];
//  ui16 Signature : 8;
//  ui16 CheckSum : 8;
//} IDENTIFY_DEVICE_DATA, *PIDENTIFY_DEVICE_DATA;








