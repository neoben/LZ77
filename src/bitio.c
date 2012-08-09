#include "../include/bitio.h"

/* Bitfile structure */
struct bitbuf
{
        int mode;               // 0 = read | 1 = write
        int fd;                 // File Descriptor
        unsigned int offset;    // First useful position in the buffer
        unsigned int nbits;     // Number of bits in the buffer
        unsigned int buffsize;  // Size of the buffer: number of bits
        char buff[0];           // Buffer
};

struct bitbuf* bitfile_open(const char *filename, int mode, unsigned int buffsize) 
{
	int fd = -1;
	struct bitbuf *bfp = NULL;

	unsigned int bytebuffsize;	// Size of the buffer: number of bytes

	bytebuffsize = buffsize % 8 ? (buffsize / 8) + 1 : (buffsize / 8);

	/* Check the buffer size */
	if(bytebuffsize < DEFAULTBUFFERSIZE) {
		bytebuffsize = DEFAULTBUFFERSIZE;
	}
	else if(bytebuffsize > MAXBUFFERSIZE) {
                bytebuffsize = MAXBUFFERSIZE;
        }

	/* Check if the file exists or if it is empty */
	if(filename == NULL || filename[0] == '\0') {
		errno = EPERM; // Operation Not Permitted
		return NULL;
	}

	/* Check that there is a correct opening mode */
	if(mode !=0 && mode !=1) {
		errno = EINVAL; // Invalid Argument
		return NULL;
	}
	
	/* Open the file */
	fd = open(filename, mode == 0 ? O_RDONLY : (O_WRONLY|O_CREAT|O_TRUNC), S_IRWXU);
	if(fd < 0) 
		return NULL;

	/* Descriptor allocation */
	bfp = malloc(sizeof(struct bitbuf) + bytebuffsize);	

	if(bfp == NULL) {
		close(fd);
		return NULL;
	}	
	
	/* Initialize the bitbuf parameters */	
	bfp->mode = mode;
	bfp->fd = fd;
	bfp->offset = 0;
	bfp->nbits = 0;
	bfp->buffsize = (bytebuffsize * 8);     // use bytebuffersize to any changes due to DEFAULTBUFFERSIZE or MAXBUFFERSIZE

	return bfp;
}

int bitfile_write(struct bitbuf* bfp, char* src, unsigned int nbits, unsigned int offset) 
{
	char* s; 			// Source buffer pointer
	uint8_t mask;			// Mask 
	unsigned int written = 0;	// Number of written bits;

	int ret;

	/* Check the parameters */
	if((bfp == NULL) || (src == NULL) || (offset > 7) || (offset < 0)) {
		errno = EINVAL; // Invalid Argument
		return -1;
	}

	/* Check if the file is in the write mode */
	if(bfp->mode != 1) {
		errno = EPERM; // Operation Not Permitted
		return -1;
	}

	s = src;
	mask = 1 << offset;	// Set the mask at the offset position

	while(nbits > 0) {
		
		/* If the bitfile buffer is full, call flush */
		if(bfp->nbits == bfp->buffsize) {
			ret = bitfile_flush(bfp);
			if(ret == -1)
				return -1;
		}

		/* Write the single bit in the bitfile buffer */
		if(*s & mask) {
			bfp->buff[(bfp->nbits / 8)] |= (1 << bfp->offset);	// write 1 
		}
		else {
			bfp->buff[(bfp->nbits / 8)] &= ~(1 << bfp->offset);	// write 0
		}	

		/* Check where to write next */
		if(mask == 0x80) { // End of the local buffer row
			s++;
			mask = 1;
		}
		else {
			mask <<= 1;
		}
		
		bfp->nbits++;
		bfp->offset = (bfp->offset + 1) % 8;

		nbits--;
		written++;	
	}

	return written;

}

int bitfile_read(struct bitbuf* bfp, char* dst, unsigned int nbits, unsigned int offset) 
{
	char* d;			// Local buffer pointer
	uint8_t bmask;			// Bitfile buffer Mask
	uint8_t mask;			// Mask
	unsigned int bread = 0;		// Nuber of read bits
	unsigned int bytebuffsize;	// Size of the bitfile buffer in bytes
	
	int ret;

	/* Check the parameters */
        if((bfp == NULL) || (offset > 7) || (offset < 0)) {
                errno = EINVAL; // Invalid Argument
                return -1;
        }

        /* Check if the file is in the read mode */
        if(bfp->mode != 0) {
                errno = EPERM; // Operation Not Permitted
                return -1;
        }
	
	d = dst;
	mask = 1 << offset;

	while(nbits > 0) {
		
		/* If the bitfile buffer is empty, read from the file */
		if(bfp->nbits == 0) {

			bfp->offset = 0;
			bytebuffsize = (bfp->buffsize / 8);

			ret = read(bfp->fd, bfp->buff, bytebuffsize);

			/* Can't read from the file */
			if(ret == -1) {
				return -1;
			}
			if(ret == 0) {
				return 0;
			}
			
			/* Numbers of bits placed in the bitfile buffer */
			bfp->nbits = ret * 8;
		}

		bmask = 1 << (bfp->offset % 8);

		/* Read the single bit from the bitfile buffer */
		if(bfp->buff[(bfp->offset / 8)] & bmask) {	
			*d |= mask;		// read 1

		}
		else {
			*d &= ~mask;		// read 0
		}
	
		/* Check where to read next */
		if(mask == 0x80) { // End of the bitfile buffer row
			mask = 1;
			d++;
		}
		else {
			mask <<= 1;
		}

		bfp->nbits--;
		bfp->offset++;		

		nbits--;
		bread++;
	}

	return bread;	
}

int bitfile_flush(struct bitbuf* bfp) 
{
	unsigned int bytestowrite;	// Number of bytes to write to the filed
	unsigned int bitstowrite;	// Number of bits remaining to write
	char lastbyte;			// Last byte to write to the file
	uint8_t lastbytemask;		// Mask for the last bitfile buffer byte

	int ret;

	/* Check the parameters */
	if(bfp == NULL) {
        	errno = EINVAL; // Invalid Argument
        	return -1;
    	}
	
	/* Check if the file is in the write mode */
        if(bfp->mode != 1) {
                errno = EPERM; // Operation Not Permitted
                return -1;
        }

	bytestowrite = bfp->nbits / 8;
	bitstowrite = bfp->nbits % 8;

	/* Write to the file 'bytestowrite' bytes from the bitfile buffer */
	if(bytestowrite != 0) {
		
		ret = write(bfp->fd, bfp->buff, bytestowrite);

		/* Can't write to the file */
		if(ret == -1) {
			return -1;
		}
	}

	/* Write to the file the last byte of the bitfile buffer:
	*  align the last bits to a byte doing 0 pdding if necessary.
	*/
	if(bitstowrite != 0) {
	
		lastbytemask = (1 << bfp->offset) - 1;
		lastbyte = bfp->buff[(bfp->nbits / 8)] & lastbytemask;

		ret = write(bfp->fd, &lastbyte, 1);

		/* Can't write to the file */
                if(ret == -1) {
                        return -1;
                }
	}		
	
	/* Reset the parameters to write in the right buffer position */
	bfp->nbits = 0;
	bfp->offset = 0;
	
	return 0;
}

int bitfile_close(struct bitbuf* bfp) 
{
	/* Check the parameters */
        if(bfp == NULL) {
                errno = EINVAL; // Invalid Argument
                return -1;
        }

	/* Write mode => flush */
	if(bfp->mode == 1) {
		bitfile_flush(bfp);
	}

	close(bfp->fd);
	free(bfp);

	return 0;
}

