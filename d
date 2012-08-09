#! /bin/sh

CMD=./lz77
I=test1.lz   	# input file path
O=test1dec.txt 	# output file path
W=1024        	# window size
L=64       	# lookahead buffer size

$CMD -d -i $I -o $O -w $W -l $L

