#include "../include/header.h"

int bit_counter(int value, int maxbits)
{
	int i;
	int temp;

	temp = value;
	i = maxbits;

	while(i > 0) {
		if((temp & (1 << (maxbits - 1))) != 0) {
			return i;
		}
		temp = temp << 1;
		i--;
	}
	return -1;
}

struct header* h_initialize(int l, int n)
{
	int app_end = 1;	// endian type | 0 = big endian, 1 = little endian

	struct header* h;

	/**** Discover the endian type ****/	
	e_c.word = 1;
	
	if(e_c.bword[0] == 1) {
		/* Set the LITTLE ENDIAN type */
		app_end = 1;
	}
	else if(e_c.bword[sizeof(e_c.word) - 1] == 1) {
		/* Set the BIG ENDIAN type */
		app_end = 0;
	}
	/**********************************/	

	h = calloc(1, sizeof(struct header));	

	if(h == NULL) {
		return NULL;
	}

	/* Initialize the header parameters */
	h->magic[0] = 1;	
	h->magic[1] = 2;	// Magic Number:
	h->magic[2] = 3;	// uniquely identifies the data type
	h->magic[3] = 4;	
	h->len = 13;		// header length (bytes)
	h->ver = 1;		// compressor version
	h->l = l;		// look ahead buffer size
	h->n = n;		// window size
	h->endian = app_end;	// endian type

	return h;
}

int h_write(struct bitbuf* bfp, struct header* h)
{
	int ret;
	int tot = 0;	// written bits

	/* Check the parameters */
	if(bfp == NULL || h == NULL) {
		errno = EINVAL; // Invalid Argument
		return -1; 
	}

	printf("%c[1m",27);
	printf("Writing the Header to the compressed file...\n");
	printf("%c[0m",27);

	/* Write the header parameters */
	ret = bitfile_write(bfp, (char*)(h->magic), 32, 0);
	if(ret == -1) {
		return -1;
	}
	else {
		printf("Magic Number: %d", (int)(h->magic[0]));
		printf("%d", (int)(h->magic[1]));
		printf("%d", (int)(h->magic[2]));
		printf("%d\n", (int)(h->magic[3]));
		tot += ret;
	}

	ret = bitfile_write(bfp, (char*)(&h->len), 8, 0);
	if(ret == -1) {
                return -1;
        }
        else {
		printf("Header Length: %d\n", (int)(h->len));
                tot += ret;
        }

	ret = bitfile_write(bfp, (char*)(&h->ver), 8, 0);
	if(ret == -1) {
                return -1;
        }
        else {
		printf("Header Version: %d\n", (int)(h->ver));
                tot += ret;
        }

	ret = bitfile_write(bfp, (char*)(&h->l), 16, 0);
	if(ret == -1) {
                return -1;
        }
        else {
		printf("Lookahead Buffer Size: %d\n", (int)(h->l));
                tot += ret;
        }

	ret = bitfile_write(bfp, (char*)(&h->n), 32, 0);
	if(ret == -1) {
                return -1;
        }
        else {
		printf("Window Size: %d\n", (int)(h->n));
                tot += ret;
        }

	ret = bitfile_write(bfp, (char*)(&h->endian), 8, 0);
	if(ret == -1) {
		return -1;
	}
	else {
		if(h->endian == 1) {
			printf("Endian Type: LITTLE ENDIAN\n");
		}
		else {
			printf("Endian Type: BIG ENDIAN\n");
		}
		tot += ret;
	}

	printf("%c[1m",27);
      	printf("........................................\n");
      	printf("%c[0m",27);

	return tot;
}

int h_read(struct bitbuf* bfp, struct header* h) 
{
	int ret;
        int tot = 0;	// read bits

        /* Check the parameters */
        if(bfp == NULL || h == NULL) {
                errno = EINVAL; // Invalid Argument
                return -1;
        }

	/* Read the header parameters */
        ret = bitfile_read(bfp, (char*)(h->magic), 32, 0);
        if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

        ret = bitfile_read(bfp, (char*)(&h->len), 8, 0);
        if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

        ret = bitfile_read(bfp, (char*)(&h->ver), 8, 0);
        if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

        ret = bitfile_read(bfp, (char*)(&h->l), 16, 0);
        if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

        ret = bitfile_read(bfp, (char*)(&h->n), 32, 0);
        if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

	ret = bitfile_read(bfp, (char*)(&h->endian), 8, 0);
	if(ret == -1) {
                return -1;
        }
        else {
                tot += ret;
        }

        return tot;
}
