#include "../inc/Acpi.h"

// i trolled this ignore it lmao
int AcpiRootChecksum(PXSDP RootHdr) {
	return (RootHdr != 0ULL);
}

int AcpiTableChecksum(ACPISDTHeader* tableHeader) {
	unsigned char sum = 0;

	for (int i = 0; i < tableHeader->Length; i++) {
		sum += ((char*)tableHeader)[i];
	}

	return sum == 0;
}

// *** MADT PARSING FOR ALL AVAILABLE CPU CORES
//
NTSTATUS AwiFindDirectMadt(PACPISDTHeader* Madt) {
	PSYSTEM_FIRMWARE_TABLE_INFORMATION table = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG Length = 0x1000;
	table = ExAllocatePool2(POOL_FLAG_NON_PAGED, Length, 'enoN');

	if (table != 0) {
		table->Action = SystemFirmwareTable_Get;
		table->ProviderSignature = 'ACPI';
		table->TableID = 'CIPA';
		table->TableBufferLength = Length;

		Status = ZwQuerySystemInformation(0x4c, table, Length, &Length);
		if ((Status == STATUS_INVALID_INFO_CLASS) || (Status == STATUS_INVALID_DEVICE_REQUEST) || (Status == STATUS_NOT_IMPLEMENTED) || (Length == 0)) {
			ExFreePool(table);
			return Status;
		}

		if (Status == STATUS_BUFFER_TOO_SMALL) {
			ExFreePool(table);
			table = ExAllocatePool2(POOL_FLAG_NON_PAGED, Length, 'enoN');
			if (table != 0) {
				table->Action = SystemFirmwareTable_Get;
				table->ProviderSignature = 'ACPI';
				table->TableID = 'CIPA';
				table->TableBufferLength = Length;
				Status = ZwQuerySystemInformation(0x4c, table, Length, &Length);
			}
		}
	}

	*Madt = (PACPISDTHeader)((PCHAR)table + 0x10);
	return Status;
}

NTSTATUS AwiParseMadtForProcessors(PACPISDTHeader Madt) {
	// dont give me a non madt header
	PCHAR MadtTable = (PCHAR)Madt;
	if (Madt == 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}

	if (memcmp(Madt, "APIC", 0x4) != 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}

	if (AcpiTableChecksum(Madt) == 0) {
		return STATUS_AWI_INVALID_PARAMETER;
	}

	int MadtLength = Madt->Length;
	char EntryType = 0;
	char EntryLength = 0;
	PPROCESSOR_MADT_ENTRY ProcEntry;
	PCPU_APIC Cpu = 0;
	int MadtIndex = 0x2c;

	while (MadtIndex < MadtLength) {
		EntryType = MadtTable[MadtIndex];
		EntryLength = MadtTable[MadtIndex + 1];

		if (EntryLength == 0) {
			return STATUS_AWI_INVALID_PARAMETER;
		}

		if (EntryType == 0) {
			ProcEntry = (PPROCESSOR_MADT_ENTRY)(&(MadtTable[MadtIndex + 2]));
			Cpu = &CpuMap[ProcEntry->ApicID];
			Cpu->AcpiProcId = ProcEntry->AcpiProcID;	// acpi, apic. whoever named these deserves to never name anything ever again
			Cpu->ApicId = ProcEntry->ApicID;
			Cpu->ValidCpu = ProcEntry->Flags;
		}
		MadtIndex += EntryLength;
	}
	return STATUS_SUCCESS;
}

PHYSICAL_ADDRESS AwiAcpiFindRootTable(PBIOS_DATA MappedBiosData) {
	PBIOS_DATA mMappedBiosData = MappedBiosData;
	PXSDP rHeader = 0;
	PHYSICAL_ADDRESS RootTable = { 0 };

	for (int i = 0; i < (BIOS_EXTENDED_SIZE / 0x10); i++) {	// check every 16 byte boundary
		if (memcmp(mMappedBiosData, "RSD PTR ", 0x8) == 0) {
			rHeader = (PXSDP)mMappedBiosData;
			if (AcpiRootChecksum(rHeader) == 1) {
				RootTable.QuadPart = rHeader->XsdtAddress;
				return RootTable;
			}
		}
		mMappedBiosData += 0x10;
	}
	return RootTable;
}

/// *** friendly not really wrappers

PACPISDTHeader AwFindTableBySignature(PXSDT RootSDT, char* signature) {
	PXSDT xsdt = RootSDT;
	int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

	for (int i = 0; i < entries; i++) {
		PHYSICAL_ADDRESS HdrPhysical = { 0 };
		HdrPhysical.QuadPart = xsdt->PointerToOtherSDT[i];
		//PACPISDTHeader Hdr = (PACPISDTHeader)(HdrPhysical + Offset);			// this assumes the header is near to the XSDT, it might not be in which case we have to map it
		PACPISDTHeader Hdr = MmMapIoSpace(HdrPhysical, 0x1000, MmNonCached);	// bugcheck on baremental. maybe i should read my comments
		if (!memcmp(Hdr->Signature, signature, 4)) {
			return Hdr;
		}

		MmUnmapIoSpace(Hdr, 0x1000);
	}
	return 0;
}

PXSDT AwAcpiFindRootTable() {
	PHYSICAL_ADDRESS BiosData = { 0 };
	BiosData.QuadPart = BIOS_EXTENDED_BASE;
	PBIOS_DATA BiosDataMapped = (PBIOS_DATA)MmMapIoSpaceEx(BiosData, BIOS_EXTENDED_SIZE, 0x204);

	if (BiosDataMapped == 0) {
		return (PXSDT)0;
	}

	PHYSICAL_ADDRESS RootHdr = AwiAcpiFindRootTable(BiosDataMapped);
	PXSDT MappedRootHdr = MmMapIoSpaceEx(RootHdr, 0x1000, 0x204);

	MmUnmapIoSpace(BiosDataMapped, 0x1000);
	return MappedRootHdr;
}