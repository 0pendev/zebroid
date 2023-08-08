#include "pe.h" 

PIMAGE_NT_HEADERS64 getNtHeaders(LPBYTE image){
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)image;
	return (PIMAGE_NT_HEADERS64)(image + dosHeader->e_lfanew);
}