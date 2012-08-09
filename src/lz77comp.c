#include "../include/lz77.h"

int compress(char* in, char* out, int n, int l)
{
	int ret;

	int l_bits;
	int n_bits;

	int tmp_match = 0;
	int byte_to_compress = 0;
	int byte_compressed = 0;
	int endfile = 0;	// endfile indicator
	int eof_flag = l + 1;	// special code for end of file sequence

	int root = UNUSED;
	int start = 0;		// pointer to the beginning of the window
	int end = 0;		// pointer to the end of the window
	int offset = 0;		// file offset (processed symbols)

	struct token t;

	FILE* f_in = NULL;
	struct bitbuf* f_out = NULL;
	struct header* h = NULL;

	char *window;
	struct node *tree;
	struct param p;

	/* Open the input file in read mode */
	f_in = fopen(in, "rb");
	if(f_in == NULL) {
		printf("Error opening the file: %s.\n", in);
		return -1;
	}	

	/* Open the output file in write mode */
	f_out = bitfile_open(out, 1, 1024);	
	if(f_out == NULL) {
		printf("Error opening the file:  %s.\n", out);
		return -1;
	}

	/* Initialize the header */
	h = h_initialize(l, n);
	if(h == NULL) {
		printf("Error initializing the header.\n");
		return -1;
	}

	/* Write the header to the output file */
	ret = h_write(f_out, h);	
	if(ret == -1) {
		printf("Error writing the header to the output file.\n");
		remove(out);
		return ret;
	}

	/* Allocate the window space */
	window = calloc((2*n) + l, sizeof(char));

	/* Allocate the tree space => 'n' tree nodes */
        tree = calloc(n, sizeof(struct node));

	init_tree(tree, n);

	/* Count the needed bits */
	n_bits = bit_counter(n, 32);
	if(n_bits == -1) {
		printf("Error counting the bits needed to the Window parameter.\n");
		return n_bits;
	}

	l_bits = bit_counter(l + 1, 16);	// Consider the EOF code = l + 1
	if(l_bits == -1) {
                printf("Error counting the bits needed to the Lookahead Buffer parameter.\n");
                return l_bits;
        }

	printf("%c[1m",27);
       	printf("Starting Compression...\n");
  	printf("%c[0m",27);
	printf("Input File: %s\n", in);
	printf("Output File: %s\n", out);

	/**** Load the data for the first time ****/	
	byte_to_compress = fread(window, 1, (2*n) + l, f_in);
      	
	if(ferror(f_in)) {
           	printf("Error loading the data in the window.\n");
              	return -1;
   	}

	if(feof(f_in)) {
		endfile = 1;
	}
	/******************************************/

	while(1) {

		while(byte_to_compress > 0) {

			/* Update the  window parameters */
                	p.start = start;
                	p.end = end;
                	p.n = n;
                	p.l = l;
                	p.offset = offset;

			t = match(window, tree, root, p);

			/* If the match length is 0 or 1 write the symbol, it is cheaper */
			if(t.match_len == 0 || t.match_len == 1) {
		
				ret = bitfile_write(f_out, (char*)(&t.match_len), l_bits, 0);
                        	if(ret == -1) {
                                	printf("Error writing the match length.\n");
                                	remove(out);
					break;
                        	}

				ret = bitfile_write(f_out, (window + end), 8, 0);
                        	if(ret == -1) {
                                	printf("Error writing the the symbol.\n");
                                	remove(out);
					break;
                        	}

                        	tmp_match = 1;
                        	byte_compressed = tmp_match;
			}

			if(t.match_len > 1) {
		
				ret = bitfile_write(f_out, (char*)(&t.match_len), l_bits, 0);
                        	if(ret == -1) {
                                	printf("Error writing the match length.\n");
                                	remove(out);
					break;
                        	}

                        	ret = bitfile_write(f_out, (char*)(&t.match_pos), n_bits, 0);
                        	if(ret == -1) {
                                	printf("Error writing the match position.\n");
                                	remove(out);
					break;
                        	}

                        	tmp_match = t.match_len;
                        	byte_compressed = tmp_match;
			}

			int i;
			for(i = 0; i < tmp_match; i++) {
		
				/* Update the window parameters */
                		p.start = start;
                		p.end = end;
                		p.n = n;
                		p.l = l;
                		p.offset = offset;

				append_node(window, tree, &root, p);
			
				/* Update the window pointers for each insertion */
				end++;
				if(end > n) {
					start++;
				}
		
				if(start == n && end == 2*n) {
			
					/* End of the data: move the window and read more data (if any) */	
					memmove(window, window + start, n + l);
					start = 0;
					end = n;

					if(endfile != 1) {
					
						byte_to_compress += fread(window + n + l, 1, n, f_in);

						if(feof(f_in)) {
							endfile = 1;
						}

						if(ferror(f_in)) {
                               				printf("Error loading the data in the window.\n");
                               				remove(out);	
 							break;                        		
						}	
					}
				}
				offset++; 
			}
			byte_to_compress -= byte_compressed;		

		}// End of while: (byte_to_compress > 0)

		if(endfile == 1) {

			/* Write the EOF code */
                        ret = bitfile_write(f_out, (char*)(&eof_flag), l_bits, 0);
                        if(ret == -1) {
                                printf("Error writing the EOF code.\n");
                                remove(out);
                                break;
                        }

                        break;
		}
		

	}// End of while: compression cicle	

	bitfile_close(f_out);
	fclose(f_in);	
	free(window);
	free(tree);

	if(ret == -1) {
		return ret;
	}

	printf("%c[1m",27);
      	printf("........................................\n");
      	printf("%c[0m",27);

	return 0;
}
