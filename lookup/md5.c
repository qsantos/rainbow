// Reference:
// RFC 1321
#include "md5.h"

#include <string.h>

static const uint8_t* padding = (uint8_t*)
	"\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
;

static const uint32_t T[] =
{
	0,

	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

static const MD5_CTX initctx = { 0, 0, {0}, 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};

void MD5Init(MD5_CTX* md5)
{
	memcpy(md5, &initctx, sizeof(MD5_CTX));
}

#define F(X,Y,Z) (((X) & (Y)) | (~(X) & (Z)))
#define G(X,Y,Z) (((X) & (Z)) | ((Y) & ~(Z)))
#define H(X,Y,Z) ((X) ^ (Y) ^ (Z))
#define I(X,Y,Z) ((Y) ^ ((X) | ~(Z)))
#define ROT(x,n) (((x) << n) | ((x) >> (32-n)))
#define OP(f,a,b,c,d,k,s,i) md5->a = md5->b + ROT(md5->a + f(md5->b,md5->c,md5->d) + X[k] + T[i], s);
void MD5Block(MD5_CTX* md5, const uint8_t block[64])
{
	uint32_t X[16];
	for (uint8_t i = 0; i < 16; i++)
		X[i] = (block[i*4] << 0) | (block[i*4+1] << 8) | (block[i*4+2] << 16) | (block[i*4+3] << 24);

	uint32_t AA = md5->A;
	uint32_t BB = md5->B;
	uint32_t CC = md5->C;
	uint32_t DD = md5->D;

	OP(F,A,B,C,D,  0, 7, 1)  OP(F,D,A,B,C,  1,12, 2)  OP(F,C,D,A,B,  2,17, 3)  OP(F,B,C,D,A,  3,22, 4)
	OP(F,A,B,C,D,  4, 7, 5)  OP(F,D,A,B,C,  5,12, 6)  OP(F,C,D,A,B,  6,17, 7)  OP(F,B,C,D,A,  7,22, 8)
	OP(F,A,B,C,D,  8, 7, 9)  OP(F,D,A,B,C,  9,12,10)  OP(F,C,D,A,B, 10,17,11)  OP(F,B,C,D,A, 11,22,12)
	OP(F,A,B,C,D, 12, 7,13)  OP(F,D,A,B,C, 13,12,14)  OP(F,C,D,A,B, 14,17,15)  OP(F,B,C,D,A, 15,22,16)

	OP(G,A,B,C,D,  1, 5,17)  OP(G,D,A,B,C,  6, 9,18)  OP(G,C,D,A,B, 11,14,19)  OP(G,B,C,D,A,  0,20,20)
	OP(G,A,B,C,D,  5, 5,21)  OP(G,D,A,B,C, 10, 9,22)  OP(G,C,D,A,B, 15,14,23)  OP(G,B,C,D,A,  4,20,24)
	OP(G,A,B,C,D,  9, 5,25)  OP(G,D,A,B,C, 14, 9,26)  OP(G,C,D,A,B,  3,14,27)  OP(G,B,C,D,A,  8,20,28)
	OP(G,A,B,C,D, 13, 5,29)  OP(G,D,A,B,C,  2, 9,30)  OP(G,C,D,A,B,  7,14,31)  OP(G,B,C,D,A, 12,20,32)

	OP(H,A,B,C,D,  5, 4,33)  OP(H,D,A,B,C,  8,11,34)  OP(H,C,D,A,B, 11,16,35)  OP(H,B,C,D,A, 14,23,36)
	OP(H,A,B,C,D,  1, 4,37)  OP(H,D,A,B,C,  4,11,38)  OP(H,C,D,A,B,  7,16,39)  OP(H,B,C,D,A, 10,23,40)
	OP(H,A,B,C,D, 13, 4,41)  OP(H,D,A,B,C,  0,11,42)  OP(H,C,D,A,B,  3,16,43)  OP(H,B,C,D,A,  6,23,44)
	OP(H,A,B,C,D,  9, 4,45)  OP(H,D,A,B,C, 12,11,46)  OP(H,C,D,A,B, 15,16,47)  OP(H,B,C,D,A,  2,23,48)

	OP(I,A,B,C,D,  0, 6,49)  OP(I,D,A,B,C,  7,10,50)  OP(I,C,D,A,B, 14,15,51)  OP(I,B,C,D,A,  5,21,52)
	OP(I,A,B,C,D, 12, 6,53)  OP(I,D,A,B,C,  3,10,54)  OP(I,C,D,A,B, 10,15,55)  OP(I,B,C,D,A,  1,21,56)
	OP(I,A,B,C,D,  8, 6,57)  OP(I,D,A,B,C, 15,10,58)  OP(I,C,D,A,B,  6,15,59)  OP(I,B,C,D,A, 13,21,60)
	OP(I,A,B,C,D,  4, 6,61)  OP(I,D,A,B,C, 11,10,62)  OP(I,C,D,A,B,  2,15,63)  OP(I,B,C,D,A,  9,21,64)

	md5->A += AA;
	md5->B += BB;
	md5->C += CC;
	md5->D += DD;

	// TODO : true clearing
}

void MD5Update(MD5_CTX* md5, const uint8_t* data, uint64_t len)
{
	uint32_t i = 0;
	uint8_t availBuf = 64 - md5->bufLen;
	if (len >= availBuf)
	{
		memcpy(md5->buffer + md5->bufLen, data, availBuf);
		MD5Block(md5, md5->buffer);
		i = availBuf;
		md5->bufLen = 0;

		while (i + 63 < len)
		{
			MD5Block(md5, data + i);
			i += 64;
		}
	}
	memcpy(md5->buffer + md5->bufLen, data + i, len - i);
	md5->bufLen += len - i;
	md5->len += len;

	// TODO : true cleaning
}

void MD5Final(MD5_CTX* md5, uint8_t dst[16])
{
	uint64_t len = md5->len << 3;
	uint8_t pad  = (md5->bufLen < 56 ? 56 : 120) - md5->bufLen;
	MD5Update(md5, padding, pad);
	MD5Update(md5, (uint8_t*) &len, 8);

	memcpy(dst +  0, &md5->A, 4);
	memcpy(dst +  4, &md5->B, 4);
	memcpy(dst +  8, &md5->C, 4);
	memcpy(dst + 12, &md5->D, 4);

	// TODO : true cleaning
}

void MD5(uint8_t dst[16], const uint8_t* src, uint64_t slen)
{
	MD5_CTX md5;
	MD5Init  (&md5);
	MD5Update(&md5, src, slen);
	MD5Final (&md5, dst);
}
