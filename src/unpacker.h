#include <windows.h>

LPBYTE ImageLoadToMemory(LPBYTE image, UINT64 imageSize);
BOOL ImageLoadImports(LPBYTE image);
BOOL ImageRelocate(LPBYTE image);
VOID ImageRunEntrypoint(PBYTE image);
