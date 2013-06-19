#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// translates 2*'count' hexadecimal digits
// from 'src' to 'count' bytes in 'dst'
void hex2bin(char* dst, const char* src, size_t count)
{
	for (; count; count--, dst++, src+=2)
		*dst = (src[0] - (src[0] <= '9' ? '0' : 87))*16 +
		       (src[1] - (src[1] <= '9' ? '0' : 87));
}

// prints 'count' bytes from 'bin' in hexadecimal
void printHexaBin(const char* bin, size_t count)
{
	for (; count; count--, bin++)
		printf("%.2x", (unsigned char) *bin);
}

// prints 'count' ASCII characters from 'str'
void printString(const char* str, size_t count)
{
	for (; count; count--, str++)
		printf("%c", *str);
}

// moves the cursor to the beginning
// of the line and erases it
inline void rewriteLine(void)
{
	printf("\r\33[K");
}

#endif
