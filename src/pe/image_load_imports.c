#include "pe.h"

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