#include <windows.h>
#include <stdio.h>

PIMAGE_NT_HEADERS64 getNtHeaders(LPBYTE image){
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)image;
	return (PIMAGE_NT_HEADERS64)(image + dosHeader->e_lfanew);
}

BOOL ImageLoadImports(LPBYTE image){
	PIMAGE_NT_HEADERS64 header = NULL;
	IMAGE_DATA_DIRECTORY directory = {0};
	PIMAGE_IMPORT_DESCRIPTOR descriptor = NULL;
	PSTR name = NULL;
	HMODULE dll = INVALID_HANDLE_VALUE;
	PIMAGE_THUNK_DATA64 lookupTable = NULL;
	PIMAGE_THUNK_DATA64 addressTable = NULL;
	FARPROC procedure = NULL;
	ULONGLONG lookupAddress = 0;
	PIMAGE_IMPORT_BY_NAME importName = NULL;

	header = getNtHeaders(image);
	directory = header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (directory.VirtualAddress == 0){
		return TRUE;
	}

	descriptor =(PIMAGE_IMPORT_DESCRIPTOR)(image + directory.VirtualAddress);
	while (descriptor->OriginalFirstThunk != 0){
		name = (PSTR)(image + descriptor->Name);
		dll = LoadLibraryA(name);
		if (dll == NULL){
			return FALSE;
		}
		lookupTable = (PIMAGE_THUNK_DATA64)(image + descriptor->OriginalFirstThunk);
		addressTable = (PIMAGE_THUNK_DATA64)(image + descriptor->FirstThunk);
		while (lookupTable->u1.AddressOfData != 0){
			procedure = NULL;
			lookupAddress = lookupTable->u1.AddressOfData;
			if ((lookupAddress & IMAGE_ORDINAL_FLAG64) != 0){
				procedure = GetProcAddress(dll, (LPCSTR)(lookupAddress & 0xFFFFFFFF));
				if (procedure == NULL) {
					return FALSE;
				}
			}
			else {
				importName = (PIMAGE_IMPORT_BY_NAME)(image + lookupAddress);
				procedure = GetProcAddress(dll, importName->Name);
				if (procedure == NULL) {
					return FALSE;
				}
			}
			addressTable->u1.Function = (ULONGLONG)procedure;
			lookupTable++;
			addressTable++;
		}
		descriptor++;
	}
	return TRUE;
}

BOOL ImageRelocate(LPBYTE image) {
	PIMAGE_NT_HEADERS64 header = getNtHeaders(image);
	IMAGE_DATA_DIRECTORY directory = {0};
	UINT_PTR imageBase = {0};
	UINT_PTR delta = {0};
	PIMAGE_BASE_RELOCATION relocationTable = NULL;
	SIZE_T relocations = 0;
	UINT16* relocationData = NULL;
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
		relocationData = (UINT16*)(&relocationTable[1]);
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

LPBYTE ImageLoadToMemory(LPBYTE image, SIZE_T imageSize) {
	LPBYTE virtualImage = VirtualAlloc(NULL, imageSize,MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (virtualImage == NULL)
		return NULL;
	memcpy(virtualImage, image, imageSize);
	return virtualImage;
}

VOID ImageRunEntrypoint(PBYTE image) {
	PIMAGE_NT_HEADERS64 header = NULL;
	VOID(*entrypoint)() = NULL;

	header = getNtHeaders(image);
	entrypoint = (VOID(*)()) (image + header->OptionalHeader.AddressOfEntryPoint);
	entrypoint();
}