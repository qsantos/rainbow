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

#ifndef RTABLE_H
#define RTABLE_H

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+l_hash+l_string (sizeofChain)

typedef unsigned char      u8;
typedef unsigned long      u32;
typedef unsigned long long u64;
typedef signed   long      s32;

typedef struct __attribute__((packed))
{
	u32 version; // RTF0 / 0x30465254
	u32 l_string;
	u32 s_reduce;
	u32 l_chains;
	u32 n_chains;
	u32 n_charset;
} RTF_header;

typedef struct
{
	u32   l_hash;      // hash length
	u32   l_string;    // string length
	char* charset;     // character set
	u32   n_charset;   // character count
	u32   s_reduce;    // reduction function seed (first index)
	u32   l_chains;    // chain length

	u32   n_chains;    // number of currently active chains
	u32   a_chains;    // space available for 'a_chains' chains
	u32   sizeofChain; // memory size of a chain
	char* chains;      // data (chain array)

	char* curstr;      // current starting point
	char* bufstr;      // temporary string
	char* bufhash;     // temporary hash
	char* bufchain;    // temporary chain
} RTable;

// generation
void RTable_New     (RTable* rt, u32 l_string, const char* charset, u32 s_reduce, u32 l_chains, u32 a_chains);
void RTable_Delete  (RTable* rt);
char RTable_AddChain(RTable* rt, const char* hash, const char* str);
char RTable_StartAt (RTable* rt, u64 index);
void RTable_Sort    (RTable* rt);

// loading and storing
void RTable_ToFile  (RTable* rt, const char* filename);
char RTable_FromFile(RTable* rt, const char* filename);

char RTable_Reverse(RTable* rt, const char* hash, char* dst);

// internal use
void RTable_Reduce(RTable* rt, u32 step, const char* hash, char* str); // hash to string reduce function

// useful functions
char index2key  (u64 index, char* key, u32 l_min, u32 l_max, const char* charset, u32 n_charset);

#endif
