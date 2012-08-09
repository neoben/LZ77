#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/lz77.h"

/* Argument to use running the program */
struct myargs 
{
	int mode;	// 0 = compression | 1 = decompression
	char* in;	// input filename
	char* out;	// output filename
	int n;		// window size
	int l;		// look ahead buffer size
};

/* Print the compressor user guide */
void print_man() 
{
	printf("%c[1m",27);
	printf("How to use lz77:\n");	
	printf("%c[0m",27);
	printf("./lz77 [options]\n\n");
	printf("%c[1m",27);
	printf("Required Options:\n");
	printf("%c[0m",27);
	printf(" -c\t\tCOMPRESSION MODE\n");	
	printf(" -d\t\tDECOMPRESSION MODE\n");
	printf(" -i [file]\tINPUT FILE PATH\n");
	printf(" -o [file]\tOUTPUT FILE PATH\n\n");
	printf("%c[1m",27);
	printf("Additional Options:\n");
	printf("%c[0m",27);
	printf(" -w [size]\tWINDOW SIZE\n");
	printf(" -l [size]\tLOOKAHEAD BUFFER SIZE\n\n");
	printf("%c[1m",27);
	printf("Run Examples:\n");
	printf("%c[0m",27);
	printf("./lz77 -c -i file.txt -o file.lz\n");
	printf("./lz77 -c -i file.txt -o file.lz -w 1024 -l 64\n");
	
	exit(0);
}

/* Parse all the necessary options */
int parse_options(struct myargs* ma, int argc, char* argv[])
{
	char c;

	/* Initializing the options */
	ma->mode = 2;
	ma->in = NULL;
	ma->out = NULL;
	ma->n = 4096;	// default value 
	ma->l = 256;	// default value

	while((c = getopt(argc, argv, "i:o:l:w:hcd")) != -1) {
                switch(c) {
			case 'c':
				ma->mode = 0;		// compression mode
				break;
			case 'd':
				ma->mode = 1;		// decompression mode
				break;
			case 'i':
				ma->in = optarg;	// input file path
				break;
			case 'o':
				ma->out = optarg;	// output file path
				break;
                        case 'l':
                             	ma->l = atoi(optarg);	// lookahead buffer size
                                break;
			case 'w':
				ma->n = atoi(optarg);	// window size
				break;
			case 'h':
				print_man();		// help
				break;
                        case '?':
                                printf("Unknow Option.\n");
				printf("Type ./lz77 -h to see the lz77 man.\n");
                                return -1;
                } 
        } 

	return 0;

}

/* Check if the options are included correctly */
int check_options(struct myargs* ma) 
{
	int ret = 0;

	switch(ma->mode) {
                case 2:
                        printf("Missing parameter. Use -c or -d option.\n");
                        printf("Type ./lz77 -h to see the lz77 man.\n");
                        return -1;
                case 0:
			if(ma->in == NULL) {
                		printf("Missing parameter. Use -i [file] option.\n");
                		printf("Type ./lz77 -h to see the lz77 man.\n");
                		return -1;
        		}
			if(ma->out == NULL) {
                                printf("Missing parameter. Use -o [file] option.\n");
                                printf("Type ./lz77 -h to see the lz77 man.\n");
                                return -1;
                        }
			if(ma->l > ma->n) {
				printf("Wrong parameters.\n");
				printf("The Lookahead Buffer Size (l) must be smalller than or equal to the Window Size (w)\n");
				printf("Type ./lz77 -h to see the lz77 man.\n");
				return -1;
			}	
	
			/* COMPRESSION */
                        ret = compress(ma->in, ma->out, ma->n, ma->l);
			if(ret == -1) {
				printf("Compression Failed!\n");
				return ret;
			}
                        break;
                case 1:
			if(ma->in == NULL) {
                                printf("Missing parameter. Use -i [file] option.\n");
                                printf("Type ./lz77 -h to see the lz77 man.\n");
                                return -1;
                        }
                        if(ma->out == NULL) {
                                printf("Missing parameter. Use -o [file] option.\n");
                                printf("Type ./lz77 -h to see the lz77 man.\n");
                                return -1;
                        }
 
			/* DECOMPRESSION */
			ret = decompress(ma->in, ma->out);
			if(ret == -1) {
                                printf("Decompression Failed!\n");
                                return ret;
                        }
                        break;
        }

	return 0;
}

int main (int argc, char* argv[])
{
	struct myargs ma;
	bzero(&ma, sizeof(struct myargs));

	parse_options(&ma, argc, argv);

	check_options(&ma);
	
	return 0;
}
