#ifndef BITIO_H
#define BITIO_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXBUFFERSIZE 1024	// byte
#define DEFAULTBUFFERSIZE 16	// byte

/* The bitfile_open function:
*  opens the file in read or write mode;
*  sets the bitfile buffer size;
*  initializes the bitfile structures parameters;
*  returns the bitfile buffer pointer.
*/
struct bitbuf* bitfile_open(const char *filename, int mode, unsigned int buffsize);

/* The bitfile_write function:
*  writes to the bitfile buffer 'nbits' bits from the buffer 'src';
*  returns the  number of written bits.
*/
int bitfile_write(struct bitbuf* bfp, char* src, unsigned int nbits, unsigned int offset);

/* The bitfile_read function:
*  reads from the bitfile buffer 'nbits' bits and puts them in the 'dst' buffer;
*  returns the number of read bits.
*/
int bitfile_read(struct bitbuf* bfp, char* dst, unsigned int nbits, unsigned int offset);

/* The bitfile_flush function:
*  writes to the file the content of the bitfile buffer.
*/
int bitfile_flush(struct bitbuf* bfp);

/* The bitfile_close function:
*  closes the file and frees the memory.
*/
int bitfile_close(struct bitbuf* bfp);

#endif
