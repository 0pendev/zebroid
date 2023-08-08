#include <windows.h>

PIMAGE_NT_HEADERS64 getNtHeaders(LPBYTE image);
LPBYTE ImageLoadToMemory(LPBYTE image, UINT64 imageSize);
BOOL ImageLoadImports(LPBYTE image);
BOOL ImageRelocate(LPBYTE image);
VOID ImageRunEntrypoint(PBYTE image);
