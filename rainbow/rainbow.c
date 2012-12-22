#include "rainbow.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "md5.h"

#define BLACK 1
#define RED   2

static RBNode* newLeaf(RBNode* parent)
{
	RBNode* ret = (RBNode*) malloc(sizeof(RBNode));
	assert(ret);
	RBNode leaf = {BLACK, parent, NULL, NULL, NULL, NULL};
	*ret = leaf;
	return ret;
}

void RBTable_New(RBTable* rbt, u32 length, const char* chars, u32 depth)
{
	rbt->root = NULL;

	rbt->hlen = 16;
	rbt->slen = length;

	rbt->charset  = strdup(chars);
	rbt->clen     = strlen(chars);
	rbt->l_chains = depth;

	rbt->bufstr1  = (char*) malloc(rbt->slen);
	rbt->bufstr2  = (char*) malloc(rbt->slen);
	rbt->bufhash  = (char*) malloc(rbt->hlen);

	assert(rbt->bufstr1);
	assert(rbt->bufstr2);
	assert(rbt->bufhash);
}

static void deleteNode(RBNode* n)
{
	if (n->left)
		deleteNode(n->left);
	if (n->right)
		deleteNode(n->right);
	free(n);
}

void RBTable_Delete(RBTable* rbt)
{
	free(rbt->bufhash);
	free(rbt->bufstr2);
	free(rbt->bufstr1);
	deleteNode(rbt->root);
}

// return NULL if key not found
RBNode* RBTable_Find(RBTable* rbt, const char* hash)
{
	RBNode* n = rbt->root;
	while (n && bstrncmp(n->hash, hash, rbt->hlen))
		n = bstrncmp(hash, n->hash, 16) < 0 ? n->left : n->right;
	return n;
}

// return NULL if key found
RBNode* RBTable_FindNew(RBTable* rbt, const char* hash)
{
	RBNode* n = rbt->root;
	while (n)
	{
		if (bstrncmp(hash, n->hash, 16) < 0)
		{
			if (n->left)
				n = n->left;
			else
				return n->left = newLeaf(n);
		}
		else if (bstrncmp(hash, n->hash, 16) > 0)
		{
			if (n->right)
				n = n->right;
			else
				return n->right = newLeaf(n);
		}
		else
			return NULL;
	}
	return rbt->root = newLeaf(NULL);
}

static void rotate_left(RBNode* p, RBNode** r)
{
	RBNode* n = p->right;

	p->right = n->left;
	n->left = p;

	if (p->right)
		p->right->parent = p;
	n->parent = p->parent;
	p->parent = n;

	*r = n;
}

static void rotate_right(RBNode* p, RBNode** r)
{
	RBNode* n = p->left;

	p->left = n->right;
	n->right = p;

	if (p->left)
		p->left->parent = p;
	n->parent = p->parent;
	p->parent = n;

	*r = n;
}

static void insert(RBTable* rbt, RBNode* n)
{
	// case 1
	if (!n->parent)
	{
		n->color = BLACK;
		return;
	}

	n->color = RED;
	RBNode* p = n->parent;

	// case 2
	if (p->color == BLACK)
		return;

	RBNode* g = p->parent;

	// case 3
	RBNode* u = p == g->left ? g->right : g->left;
	if (u && u->color == RED)
	{
		p->color = BLACK;
		u->color = BLACK;
//		g->color = RED; // TODO
		insert(rbt, g);
		return;
	}

	// case 4
	if (n == p->right && p == g->left)
	{
		rotate_left(p, &g->left);

		n = n->left;
		p = n->parent;
	}
	else if (n == p->left && p == g->right)
	{
		rotate_right(p, &g->right);

		n = n->right;
		p = n->parent;
	}


	RBNode** r = g->parent ? (g->parent->left == g ? &g->parent->left : &g->parent->right) : &rbt->root;

	p->color = BLACK;
	g->color = RED;
	if (n == p->left)
		rotate_right(g, r);
	else
		rotate_left(g, r);
}

char RBTable_AddChain(RBTable* rbt, const char* hash, const char* str)
{
	RBNode* n = RBTable_FindNew(rbt, hash);

	if (n)
	{
		insert(rbt, n);
		n->hash = bstrndup(hash, rbt->hlen);
		n->str  = bstrndup(str,  rbt->slen);
		return 1;
	}
	return 0;
}

char RBTable_FindChain(RBTable* rbt)
{
	// pick a starting point
	for (u32 i = 0; i < rbt->slen; i++)
		rbt->bufstr1[i] = rbt->charset[random() % rbt->clen];

	// start a new chain from 'str'
	MD5((uint8_t*) rbt->bufhash, (uint8_t*) rbt->bufstr1, rbt->slen);
	for (u32 step = 1; step < rbt->l_chains; step++)
	{
		RBTable_Mask(rbt, step, rbt->bufhash, rbt->bufstr2); // TODO
		MD5((uint8_t*) rbt->bufhash, (uint8_t*) rbt->bufstr2, rbt->slen);
	}

	return RBTable_AddChain(rbt, rbt->bufhash, rbt->bufstr1);
}

char RBTable_Reverse(RBTable* rbt, const char* hash, char* dst)
{
	// test for every distance to the end point
	for (u32 firstStep = rbt->l_chains; firstStep >= 1; firstStep--)
	{
		// get the end point hash
		memcpy(rbt->bufhash, hash, rbt->hlen);
		for (u32 step = firstStep; step < rbt->l_chains; step++)
		{
			RBTable_Mask(rbt, step, rbt->bufhash, rbt->bufstr1);
			MD5((uint8_t*) rbt->bufhash, (uint8_t*) rbt->bufstr1, rbt->slen);
		}

		// find the hash's chain
		RBNode* n = RBTable_Find(rbt, rbt->bufhash);
		if (!n)
			continue;

		// get the previous string
		memcpy(rbt->bufstr1, n->str, rbt->slen);
		MD5((uint8_t*) rbt->bufhash, (uint8_t*) rbt->bufstr1, rbt->slen);
		u32 step = 1;
		while (step < rbt->l_chains && bstrncmp(rbt->bufhash, hash, rbt->hlen) != 0)
		{
			RBTable_Mask(rbt, step++, rbt->bufhash, rbt->bufstr1);
			MD5((uint8_t*) rbt->bufhash, (uint8_t*) rbt->bufstr1, rbt->slen);
		}
		if (step < rbt->l_chains)
		{
			if (dst)
				memcpy(dst, rbt->bufstr1, rbt->slen);
			return 1;
		}
	}
	return 0;
}

void RBTable_Mask(RBTable* rbt, u32 step, const char* hash, char* str)
{
	for (u32 j = 0; j < rbt->slen; j++, str++, hash++)
		*str = rbt->charset[(u8)(*hash ^ step) % rbt->clen];
}

void dump(RBTable* rbt, RBNode* n, FILE* f)
{
	if (!n) return;

	dump(rbt, n->left, f);

	fwrite(n->hash, 1, rbt->hlen, f);
	fwrite(n->str,  1, rbt->slen, f);

	dump(rbt, n->right, f);
}

u32 RBTable_FromFile(RBTable* rbt, FILE* f)
{
	u32 ret = 0;
	while (1)
	{
		fread(rbt->bufhash, 1, rbt->hlen, f);
		if (feof(f))
			break;
		fread(rbt->bufstr1, 1, rbt->slen, f);

		ret += RBTable_AddChain(rbt, rbt->bufhash, rbt->bufstr1);
	}
	return ret;
}

void RBTable_ToFile(RBTable* rbt, FILE* f)
{
	dump(rbt, rbt->root, f);
}

u32 RBTable_FromFileN(RBTable* rbt, const char* filename)
{
	FILE* f = fopen(filename, "r");
	if (!f)
		return 0;
	u32 ret = RBTable_FromFile(rbt, f);
	fclose(f);
	printf("%lu chains loaded from '%s'\n", ret, filename);
	return ret;
}

void RBTable_ToFileN(RBTable* rbt, const char* filename)
{
	FILE* f = fopen(filename, "w");
	if (!f)
		return;
	RBTable_ToFile(rbt, f);
	fclose(f);
}

char bstrncmp(const char* a, const char* b, int n)
{
	for (int i = 0; i < n; i++, a++, b++)
		if (*a != *b)
			return *(u8*)a < *(u8*)b ? -1 : 1;
	return 0;
}

char* bstrndup(const char* str, u32 len)
{
	char* ret = malloc(len);
	assert(ret);
	memcpy(ret, str, len);
	return ret;
}

void hex2hash(const char* hex, char* hash, u32 hlen)
{
	for (u32 i = 0; i < hlen; i++)
	{
		*hash  = *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		*hash *= 16;
		*hash += *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		hash++;
	}
}

void printHash(const char* hash, u32 hlen)
{
	for (u32 i = 0; i < hlen; i++, hash++)
		printf("%.2x", (u8) *hash);
}

void printString(const char* str, u32 slen)
{
	for (u32 j = 0; j < slen; j++, str++)
		printf("%c", *str);
}
