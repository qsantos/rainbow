/*\
 *  Implementation of rainbow tables for hash cracking
 *  Copyright (C) 2012-2013 Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

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
