#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static unsigned int HashFun(const char* str, unsigned int len);

HashTable* HashTable_New(int size)
{
	HashTable* ret = (HashTable*)malloc(sizeof(HashTable));
	int mem = sizeof(KValue) * size;
	ret->t = (KValue*)malloc(mem);
	ret->size = size;
	memset(ret->t, 0, mem);
	return ret;
}

void HashTable_Delete(HashTable* ht)
{
	assert(ht);
	
	for (int i = 0; i < ht->size; i++)
		free(ht->t[i].k);
	free(ht->t);
	free(ht);
}

char HashTable_Exists(HashTable* ht, char* key)
{
	assert(ht);
	
	int cur = HashFun(key, strlen(key)) % ht->size;
	while (ht->t[cur].k && strcmp(ht->t[cur].k, key))
		if (++cur >= ht->size)
			cur = 0;
	return ht->t[cur].k != NULL;
}

int HashTable_Find(HashTable* ht, char* key)
{
	assert(ht);
	
	int l = strlen(key);
	int cur = HashFun(key, l) % ht->size;
	while (ht->t[cur].k && strcmp(ht->t[cur].k, key))
		if (++cur >= ht->size)
			cur = 0;
	if (!ht->t[cur].k)
	{
		ht->t[cur].k = (char*)malloc(l + 1);
		strcpy(ht->t[cur].k, key);
		ht->t[cur].v = ht->n_elements++;
	}
	return ht->t[cur].v;
}

/*
**************************************************************************
*                                                                        *
*          General Purpose Hash Function Algorithms Library              *
*                                                                        *
* Author: Arash Partow - 2002                                            *
* URL: http://www.partow.net                                             *
* URL: http://www.partow.net/programming/hashfunctions/index.html        *
*                                                                        *
* Copyright notice:                                                      *
* Free use of the General Purpose Hash Function Algorithms Library is    *
* permitted under the guidelines and in accordance with the most current *
* version of the Common Public License.                                  *
* http://www.opensource.org/licenses/cpl1.0.php                          *
*                                                                        *
**************************************************************************
*/
static unsigned int HashFun(const char* str, unsigned int len)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}
