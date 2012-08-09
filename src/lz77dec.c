#include "../include/lz77.h"

/* Endianess structure, used to discover the endian type */
union endianess_d {
        unsigned long word;
        char bword[sizeof(unsigned long)];
} e_d;

int decompress(char *in, char *out) {

	int ret;

	int l;
	int n;
	int len;
	int ver;
	int endian;
	int curr_end = 1;
	int magic[4];
	magic[0] = 1;
	magic[1] = 2;
	magic[2] = 3;
	magic[3] = 4;

	int l_bits;
	int n_bits;

	char symbol;
	int eof_flag;
	int match_len = 0;
	int match_pos = 0;
	int next_match = 0;	

	int start = 0;
	int end = 0;

	FILE* f_out = NULL;
	struct bitbuf* f_in = NULL;
	
	char* window;
	struct header h;

	/* Open the input file in read mode */
	f_in = bitfile_open(in, 0, 1024);
	if(f_in == NULL) {
		printf("Error opening the file: %s\n", in);
		return -1;
	}
	
	/* Open the output file in write mode*/
	f_out = fopen(out,"w");
	if(f_out == NULL) {
		printf("Error opening the file: %s\n", out);
		return -1;
	}

	/* Read the header from the input file */
	ret = h_read(f_in, &h);
	if(ret == -1) {
		printf("Error reading the header from the input file\n");
		return ret;
	}

	/* Take the compressor parameters from the header */
	n = h.n;
	l = h.l;
	len = h.len;
	ver = h.ver;
	endian = h.endian;

	eof_flag = l + 1;

	/* Allocate the window space */
        window = calloc((2*n) + l, sizeof(char));

	/* Count the needed bits */
	n_bits = bit_counter(n, 32);
        if(n_bits == -1) {
                printf("Error counting the bits needed to the Window parameter.\n");
                return n_bits;
        }

        l_bits = bit_counter(l + 1, 16);        // Consider the EOF code = l + 1
        if(l_bits == -1) {
                printf("Error counting the bits needed to the Lookahead Buffer parameter.\n");
                return l_bits;
        }

	/* Check the compatibility, use the magic number */
	int i;
	for(i = 0; i < 4; i++) {
		if(magic[i] != h.magic[i]) {
			printf("Error: the compressor is not compatible.\n");
			return -1;
		}
	}

	printf("%c[1m",27);
        printf("Reading the Header from the compressed file...\n");
        printf("%c[0m",27);
	printf("Magic Number: ");
	for(i = 0; i < 4; i++) {
		printf("%d", magic[i]);
	}
	printf("\n");
	printf("Header Length: %d\n", len);
	printf("Header Version: %d\n", ver);
	printf("Lookahead Buffer Size: %d\n", l);
	printf("Window Size: %d\n", n);
	if(endian == 1) {
     		printf("Endian Type: LITTLE ENDIAN\n");
     	}
     	else {
     		printf("Endian Type: BIG ENDIAN\n");
	}
	printf("%c[1m",27);
        printf("........................................\n");
        printf("%c[0m",27);

	/**** Discover the endian type ****/
        e_d.word = 1;

        if(e_d.bword[0] == 1) {
                /* Set the LITTLE ENDIAN type */
                curr_end = 1;
        }
        else if(e_d.bword[sizeof(e_d.word) - 1] == 1) {
                /* Set the BIG ENDIAN type */
                curr_end = 0;
        } 
        /**********************************/

	printf("%c[1m",27);
        printf("Starting Decompression...\n");
        printf("%c[0m",27);
	printf("Input File: %s\n", in);
	printf("Output File: %s\n", out);
	if(curr_end == 1) {
                printf("Current Endian Type: LITTLE ENDIAN\n");
        }
        else {
                printf("Current Endian Type: BIG ENDIAN\n");
        }

	while(1) {

		ret = bitfile_read(f_in, (char*)(&match_len), l_bits, 0);
		if(ret == -1) {
			printf("Error reading the match length.\n");
			remove(out);
			break;
		}

		/* Not the same endian type: do the endian conversion */
        	if(curr_end != endian) {
			
			/* Conversion from LITTLE ENDIAN to BIG ENDIAN*/
			if(curr_end == 1) {
				match_len = htonl(match_len);
			} 
			/* Conversion from BIG ENDIAN to LITTLE ENDIAN */
			else {
				match_len = ntohl(match_len);
			}
       		}

		if(match_len == eof_flag) {
			break;
		}

		else if(match_len == 0 || match_len == 1) {

			ret = bitfile_read(f_in, (char*)(&symbol), 8, 0);
			if(ret == -1) {
				printf("Error reading the symbol.\n");
				remove(out);
				break;
			}	
			
			/* Fill the window with the read symbol starting from the end */
			window[end] = symbol;

			ret = fwrite(&symbol, 1, 1, f_out);
			if(ret == -1) {
				printf("Error writing the symbol to the file.\n");
				remove(out);
				break;
			}

			end++;
                        if(end > n) {
                                start++;
				next_match = 0;
                        }

			if(start == n && end == 2*n) {
			
				/* End of the window: move the data */	
				memmove(window, window + start, n);
				start = 0;
				end = n;
			}
		}

		else if(match_len > 1) {
			
			ret = bitfile_read(f_in, (char*)(&match_pos), n_bits, 0);
			if(ret == -1) {
				printf("Error reading the match position.\n");
				remove(out);
				break;
			}
	
			/* Not the same endian type: do the endian conversion */
                	if(curr_end != endian) {
			
				/* Conversion from LITTLE ENDIAN to BIG ENDIAN*/
                        	if(curr_end == 1) {
                                	match_pos = htonl(match_pos);
                        	}
                        	/* Conversion from BIG ENDIAN to LITTLE ENDIAN */
                        	else {
                                	match_pos = ntohl(match_pos);
                        	}
                	}

			int i;
			for(i = 0; i < match_len; i++) {
				
				if(end > n) {
					memmove(window + end, window + start + match_pos, 1);
				}
				else {
					memmove(window + end, window + start + match_pos + next_match, 1);
				}
			
				ret = fwrite(window + end, 1, 1, f_out);
				if(ret == -1) {
                                	printf("Error writing the symbol to the file.\n");
                                	remove(out);
                                	break;
                        	}

				end++;
				next_match++;

				if(end > n) {
			
					start++;
					
					/* Management of a borderline case: 
					*  the 'end' pointer has just exceeded the 'n' value;
					*  the 'start' pointer has just exceeded the 0 value.
					*  SET PROPRERLY THE POINTERS TO CATCH THE RIGHT WINDOW SECTION 
					*/	
					if(start == 1) {
						match_pos = match_pos + next_match - 1;
					}
				
					next_match = 0;
				}

				if(start == n && end == 2*n) {

					memmove(window, window + start, n);
					start = 0;
					end = n;
				}
			}

			next_match = 0;
		}
	}

	bitfile_close(f_in);
        fclose(f_out);
        free(window);

	if(ret == -1) {
		return ret;
	}

        printf("%c[1m",27);
        printf("........................................\n");
        printf("%c[0m",27);

	return 0;
	}

