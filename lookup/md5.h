#ifndef MD5_H
#define MD5_H

// MD5 provides a 16 byte hash
#include <stdint.h>

typedef struct
{
	uint64_t len;
	uint8_t  bufLen;
	uint8_t  buffer[64];
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
} MD5_CTX;

void MD5Init  (MD5_CTX* md5);
void MD5Block (MD5_CTX* md5, const uint8_t block[64]);
void MD5Update(MD5_CTX* md5, const uint8_t* data, uint64_t len);
void MD5Final (MD5_CTX* md5, uint8_t dst[16]);

void MD5(uint8_t dst[16], const uint8_t* src, uint64_t slen);

#endif
