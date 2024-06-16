#pragma once
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#include <ntddk.h>

typedef INT8 uint8_t;
typedef INT16 uint16_t;
typedef INT32 uint32_t;
typedef INT64 uint64_t;

typedef struct _LAPIC_REGISTER {
    INT32 Register;
    char DUMMYDATA[0xc];
} LAPIC_REGISTER;

typedef union _INTERRUPT_COMMAND_REGISTER_LOW {
    struct {
        unsigned int Vector : 8;
        unsigned int DeliveryMode : 3;
        unsigned int DestinationMode : 1;
        unsigned int DeliveryStatus : 1;
        unsigned int RSRVD : 1;
        unsigned int Level : 1;
        unsigned int TriggerMode : 1;
        unsigned int Reserved_0 : 2;
        unsigned int Destination : 2;
        unsigned int Reserved_1 : 12;

        char DUMMYDATA[0xc];
    } u;
    unsigned int AsUInt;
    char DUMMYDATA[0xc];
} INTERRUPT_COMMAND_REGISTER_LOW, *P_INTERRUPT_COMMAND_REGISTER_LOW;

typedef union _INTERRUPT_COMMAND_REGISTER_HIGH {
    struct {
        unsigned int Reserved : 24;
        unsigned int Destination : 8;
        char DUMMYDATA[0xc];
    } u;
    unsigned int AsUInt;
    char DUMMYDATA[0xc];
} INTERRUPT_COMMAND_REGISTER_HIGH, * PINTERRUPT_COMMAND_REGISTER_HIGH;

typedef struct _INTERRUPT {
    INTERRUPT_COMMAND_REGISTER_LOW Low;
    INTERRUPT_COMMAND_REGISTER_HIGH High;
} INTERRUPT, *PINTERRUPT;

typedef CHAR BIOS_DATA, *PBIOS_DATA;

typedef union _APIC_BASE_MSR {
    struct {
        UINT64 Reserved_0 : 8;
        UINT64 BootstrapCPU : 1;
        UINT64 Reserved_1 : 1;
        UINT64 x2APICEnabled : 1;
        UINT64 APICEnabled : 1;
        UINT64 APICBase : 40;
        UINT64 Reserved_2 : 12;
    } u;
    UINT64 AsUInt;
} APIC_BASE_MSR;

PACK(typedef struct _PROCESSOR_MADT_ENTRY {
    char AcpiProcID;
    char ApicID;
    int Flags;
}) PROCESSOR_MADT_ENTRY, *PPROCESSOR_MADT_ENTRY;

PACK(typedef struct _RSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;      // deprecated since version 2.0
}) RSDP, * RXSDP;

PACK(typedef struct _XSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;      // deprecated since version 2.0

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
}) XSDP, *PXSDP;

typedef struct _ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} ACPISDTHeader, *PACPISDTHeader;

PACK(typedef struct _XSDT {
    ACPISDTHeader h;
    uint64_t PointerToOtherSDT[ANYSIZE_ARRAY];
}) XSDT, *PXSDT;

typedef struct _LAPIC {
    char Reserved_0[0x20];

    LAPIC_REGISTER LAPICID;
    LAPIC_REGISTER LAPICVER;

    char Reserved_1[0x40];

    LAPIC_REGISTER TPR;
    LAPIC_REGISTER APR;
    LAPIC_REGISTER PPR;
    LAPIC_REGISTER EOI;
    LAPIC_REGISTER RRD;
    LAPIC_REGISTER LDR;
    LAPIC_REGISTER DFR;
    LAPIC_REGISTER SPURIOUS_INTERRUPT;

    LAPIC_REGISTER INSERVICE[0x8];
    LAPIC_REGISTER TRIGGERTMR[0x8];
    LAPIC_REGISTER INTERRUPTREQUEST[0x8];

    LAPIC_REGISTER ErrorStatus;

    char Reserved_2[0x60];

    LAPIC_REGISTER CMCI;

    INTERRUPT_COMMAND_REGISTER_LOW InterruptCommandLow;
    INTERRUPT_COMMAND_REGISTER_HIGH InterruptCommandHigh;

    LAPIC_REGISTER LvtTimer;
    LAPIC_REGISTER LvtThermal;
    LAPIC_REGISTER LvtPerformance;
    LAPIC_REGISTER LvtLINT0;
    LAPIC_REGISTER LvtLINT1;
    LAPIC_REGISTER LvtErr;
    LAPIC_REGISTER InitCount;
    LAPIC_REGISTER CurrentCount;
    
    char Reserved_3[0x40];

    LAPIC_REGISTER DivideConf;

    char Reserved_4[0x10];
} LAPIC, *PLAPIC;

typedef struct _mp_floating_pointer_structure {
    char signature[4];
    uint32_t configuration_table;
    uint8_t length; // In 16 bytes (e.g. 1 = 16 bytes, 2 = 32 bytes)
    uint8_t mp_specification_revision;
    uint8_t checksum; // This value should make all bytes in the table equal 0 when added together
    uint8_t default_configuration; // If this is not zero then configuration_table should be 
    // ignored and a default configuration should be loaded instead
    uint32_t features; // If bit 7 is then the IMCR is present and PIC mode is being used, otherwise 
    // virtual wire mode is; all other bits are reserved
} FLOATING_POINTER_STRUCTURE, * PFLOATING_POINTER_STRUCTURE;

typedef struct _mp_configuration_table {
    char signature[4]; // "PCMP"
    uint16_t length;
    uint8_t mp_specification_revision;
    uint8_t checksum; // Again, the byte should be all bytes in the table add up to 0
    char oem_id[8];
    char product_id[12];
    uint32_t oem_table;
    uint16_t oem_table_size;
    uint16_t entry_count; // This value represents how many entries are following this table
    uint32_t lapic_address; // This is the memory mapped address of the local APICs 
    uint16_t extended_table_length;
    uint8_t extended_table_checksum;
    uint8_t reserved;
} CONFIGURATION_TABLE, * PCONFIGURATION_TABLE;

typedef struct _entry_processor {
    uint8_t type; // Always 0
    uint8_t local_apic_id;
    uint8_t local_apic_version;
    uint8_t flags; // If bit 0 is clear then the processor must be ignored
    // If bit 1 is set then the processor is the bootstrap processor
    uint32_t signature;
    uint32_t feature_flags;
    uint64_t reserved;
} ENTRY_PROCESSOR, * PENTRY_PROCESSOR;

typedef struct _entry_io_apic {
    uint8_t type; // Always 2
    uint8_t id;
    uint8_t version;
    uint8_t flags; // If bit 0 is set then the entry should be ignored
    uint32_t address; // The memory mapped address of the IO APIC is memory
} IO_APIC, * PIO_APIC;

typedef PVOID(*pRtlPcToFileHeader)(PVOID, PVOID);
typedef PVOID(*pRtlFindExportedRoutineByName)(PVOID, PCHAR);
