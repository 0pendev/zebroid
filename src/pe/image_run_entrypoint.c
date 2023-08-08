#include "pe.h"

VOID ImageRunEntrypoint(PBYTE image) {
	PIMAGE_NT_HEADERS64 header = NULL;
	VOID(*entrypoint)() = NULL;

	header = getNtHeaders(image);
	entrypoint = (VOID(*)()) (image + header->OptionalHeader.AddressOfEntryPoint);
	entrypoint();
}