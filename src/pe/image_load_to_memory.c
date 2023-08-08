#include "pe.h"

LPBYTE ImageLoadToMemory(LPBYTE image, SIZE_T imageSize) {
	LPBYTE virtualImage = VirtualAlloc(NULL, imageSize,MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (virtualImage == NULL)
		return NULL;
	memcpy(virtualImage, image, imageSize);
	return virtualImage;
}