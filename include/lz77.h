#ifndef LZ77_H
#define LZ77_H

#include <arpa/inet.h>
#include "header.h"
#include "bitio.h"
#include "treemanager.h"

/* The compress function:
*  performs the compression operations.
*  @param in: input file path;
*  @param out: output file path;
*  @param n: window size;
*  @param l: lookahead buffer size. 
*/
int compress(char* in, char* out, int n, int l);

/* The decompress function:
*  performs the decompression operations.
*  @param in: input file path;
*  @param out: output file path.
*/
int decompress(char* in, char *out);

#endif
