#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct
{
	char* k;
	int v;
} KValue;

typedef struct
{
	KValue* t;
	int size;
	int n_elements;
} HashTable;

HashTable* HashTable_New   (int);
void       HashTable_Delete(HashTable*);
char       HashTable_Exists(HashTable*, char*);
int        HashTable_Find  (HashTable*, char*);

#endif
