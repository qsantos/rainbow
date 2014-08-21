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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#include "md5.h"
#include "crack.h"
#include "utils.h"

#define ERROR(...)                    \
{                                     \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n");        \
	usage(argc, argv);            \
	exit(1);                      \
}

static char preload = 0; // active thread table preloading ?

// some variables used internally by reverseHashes()
static u32     n_rt   = 0;    // the number of available tables
static char**  files  = NULL; // the table file names
static RTable* rt     = NULL; // the table being loaded
static u32     cur_f  = 0;    // the identifier of the table being loaded
static char*   bufstr = NULL; // some string buffer

static RTable rt1;
static RTable rt2;
static void* prepareNextTable(void* param)
{
	(void) param;
	rt = rt == &rt1 ? &rt2 : &rt1;
	RTable_FromFile(rt, files[cur_f++]);
	return NULL;
}

// hashes have 16 byte to decribe the value
// and one for the status (pending or solved)
// if verbose is set, print each reversed hash
// otherwise, just display a progress status
static void reverseHashes(char** hashes, size_t count, char verbose)
{
	// preload first table
	pthread_t prepThread;
	if (preload)
		pthread_create(&prepThread, NULL, prepareNextTable, NULL);

	size_t done = 0;
	cur_f = 0;
	printf("0 / %zu", count);
	for (u32 i = 0; i < n_rt; i++)
	{
		RTable* loaded;
		if (preload)
		{
			// wait for current table to be loaded and
			// initialize the loading of the next one
			pthread_join(prepThread, NULL);
			loaded = rt;
			if (i < n_rt-1)
				pthread_create(&prepThread, NULL, prepareNextTable, NULL);
		}
		else
		{
			RTable_FromFile(&rt1, files[i]);
			loaded = &rt1;
		}

		// use the current table
		for (size_t j = 0; j < count; j++)
		{
			char* hash = hashes[j];
			if (hash[16])
				continue;

			char res = RTable_Reverse(loaded, hash, bufstr);
			if (res)
			{
				hash[16] = 1;
				done++;
				rewriteLine();
				if (verbose)
				{
					printHexaBin(hash, 16);
					printf(" ");
					printString(bufstr, loaded->l_string);
					printf("\n");
				}
				else
				{
					printf("%zu / %zu", done, count);
					fflush(stdout);
				}
			}
		}
		RTable_Delete(loaded);

		// wait for the thread to finish and free
		// the unused table before returning
		if (done == count)
		{
			if (preload && i < n_rt-1)
			{
				pthread_join(prepThread, NULL);
				RTable_Delete(rt);
			}
			break;
		}
	}
	if (!verbose)
		printf("\n");
}

static void usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s [OPTIONS] TARGET src1 [src2 [...]]\n"
		"Try and reverse hashes\n"
		"\n"
		"OPTIONS:\n"
		" -p, --preload    preload next table during computation\n"
		"                  faster for long chains, uses twice as much of memory\n"
		"\n"
		"TARGET:\n"
		" -x, --hash HASH  sets the target to HASH\n"
		" -f, --file FILE  read the targets from FILE (- for stdin, one per line)\n"
		" -r, --random N   targets are N random string hashes (benchmarking)\n"
		,
		argv[0]
	);
}

typedef enum
{
	T_HASH,
	T_FILE,
	T_RAND,
} Target;

int main(int argc, char** argv)
{
	if (argc == 1 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		usage(argc, argv);
		exit(0);
	}
	else if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
	{
		printf("rtcrack\n");
		printf("Compiled on %s at %s\n", __DATE__, __TIME__);
		exit(0);
	}

	int argn = 1;

	// OPTIONS
	char* ostr = argv[argn];
	if (strcmp(ostr, "-p") == 0 || strcmp(ostr, "--preload") == 0)
	{
		preload = 1;
		argn++;
	}

	// TARGET
	Target ttype;
	char* tstr = argv[argn++];
	if (strcmp(tstr, "-x") == 0 || strcmp(tstr, "--hash") == 0)
		ttype = T_HASH;
	else if (strcmp(tstr, "-f") == 0 || strcmp(tstr, "--file") == 0)
		ttype = T_FILE;
	else if (strcmp(tstr, "-r") == 0 || strcmp(tstr, "--random") == 0)
		ttype = T_RAND;
	else
		ERROR("Invalid target '%s'\n", tstr);

	char* tparam = argv[argn++];

	// table list
	if (argn >= argc)
	{
		usage(argc, argv);
		exit(1);
	}
	n_rt  = argc-argn;
	files = argv + argn;

	// some parameters
	// TODO
	RTable_FromFile(&rt1, files[0]);
	u32   l_string  = rt1.l_string;
	char* charset   = rt1.charset;
	u32   n_charset = rt1.n_charset;
	RTable_Delete(&rt1);

	// some share variables
	size_t n_hashes;
	char** hashes;
	bufstr = malloc(l_string);
	assert(bufstr);

	// try and crack hash(es)
	switch (ttype)
	{
	case T_HASH:
		(void) 0;
		char* hash = (char*) malloc(17);;
		assert(hash);
		hex2bin(hash, tparam, 16);
		hash[16] = 0; // pending

		reverseHashes(&hash, 1, 1);

		free(hash);
		break;
	case T_FILE:
		(void) 0;
		FILE* f = strcmp(tparam, "-") == 0 ? stdin : fopen(tparam, "r");
		assert(f);

		n_hashes = 0;
		size_t a_hashes = 1;
		hashes = (char**) malloc(sizeof(char*));
		assert(hashes);

		char* line = NULL;
		size_t n_line = 0;
		while (1)
		{
			getline(&line, &n_line, f);
			if (feof(f))
				break;

			if (line[0] == '#') // ignore comments
				continue;

			if (strlen(line) < 33) // ignore too short lines
				continue;

			if (n_hashes >= a_hashes)
			{
				a_hashes *= 2;
				hashes = (char**) realloc(hashes, sizeof(char*) * a_hashes);
				assert(hashes);
			}

			char* hash = (char*) malloc(17);
			assert(hash);
			hex2bin(hash, line, 16);
			hash[16] = 0; // pending
			hashes[n_hashes++] = hash;
		}
		fclose(f);

		reverseHashes(hashes, n_hashes, 1);

		for (size_t i = 0; i < n_hashes; i++)
			free(hashes[i]);
		free(hashes);
		break;
	case T_RAND:
		srandom(time(NULL));
		n_hashes = atoi(tparam);
		hashes = (char**) malloc(sizeof(char*) * n_hashes);
		assert(hashes);
		for (size_t i = 0; i < n_hashes; i++)
		{
			char* hash = hashes[i] = (char*) malloc(17);
			for (size_t j = 0; j < l_string; j++)
				bufstr[j] = charset[random() % n_charset];
			MD5((u8*) hash, (u8*) bufstr, l_string);
			hash[16] = 0; // pending
		}

		reverseHashes(hashes, n_hashes, 0);

		for (size_t i = 0; i < n_hashes; i++)
			free(hashes[i]);
		free(hashes);
		break;
	}
	free(bufstr);

	return 0;
}
