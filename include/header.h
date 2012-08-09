#ifndef HEADER_H
#define HEADER_H

#include "bitio.h"

/* Header structure */
struct header 
{
	uint8_t magic[4];	// magic number
	uint8_t len;		// header length (bytes)
	uint8_t ver;		// compressor version
	uint16_t l;		// lookahead buffer size
	uint32_t n;		// window size
	uint8_t endian;		// endian type | 0 = big endian, 1 = little endian
};

/* Endianess structure, used to discover the endian type */
union endianess_c {
	unsigned long word;
	char bword[sizeof(unsigned long)];
} e_c;

/* The bit_counter function: 
*  counts the number of bits required to write the value 'value';
*  returns the number of bits.
*/
int bit_counter(int value, int maxbits);

/* The h_initialize function:
*  initializes the header with the right parameters using the included options;
*  returns the pointer to the initialized header structure.
*  @param l: lookahead buffer size;
*  @param n: window size.
*/
struct header* h_initialize(int l, int n);

/* The h_write function:
*  writes the header using the bitio library;
*  returns the number of written bits.
*/
int h_write(struct bitbuf* bfp, struct header* h);

/* The h_read function:
*  reads the header using the bitio library.
*  returns the number of read bits.
*/
int h_read(struct bitbuf* bfp, struct header* h);

#endif
