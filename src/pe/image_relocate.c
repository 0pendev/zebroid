#include "pe.h"

BOOL ImageRelocate(LPBYTE image) {
	PIMAGE_NT_HEADERS64 header = getNtHeaders(image);
	IMAGE_DATA_DIRECTORY directory = {0};
	UINT_PTR imageBase = {0};
	UINT_PTR delta = {0};
	PIMAGE_BASE_RELOCATION relocationTable = NULL;
	SIZE_T relocations = 0;
	PUINT16 relocationData = NULL;
	UINT16 relocation = 0;
	UINT16 type = 0;
	UINT16 offset = 0;
	PUINT_PTR ptr = NULL;

	if ((header->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) == 0){
		return FALSE;
	}
	directory = header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (directory.VirtualAddress == 0) {
		return FALSE;
	}

	imageBase = header->OptionalHeader.ImageBase;
	delta = (UINT_PTR)image - imageBase;
	relocationTable = (PIMAGE_BASE_RELOCATION)(image + directory.VirtualAddress);
	
	while (relocationTable->VirtualAddress != 0){
		relocations = (relocationTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(UINT16);
		relocationData = (PUINT16)(&relocationTable[1]);
		for (SIZE_T i=0; i<relocations; ++i){
			relocation = relocationData[i];
			type = relocation >> 12;
			offset = relocation & 0xFFF;
			ptr = (PUINT_PTR)(image + relocationTable->VirtualAddress + offset);
			if (type == IMAGE_REL_BASED_DIR64)
				*ptr += delta;
		}
		relocationTable = (PIMAGE_BASE_RELOCATION)(
			(LPBYTE)(relocationTable) + relocationTable->SizeOfBlock
			);
	}
	return TRUE;
}