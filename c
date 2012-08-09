#! /bin/sh

CMD=./lz77
I=test1.txt	# input file path
O=test1.lz	# output file path
W=1024		# window size
L=64		# lookahead buffer size

$CMD -c -i $I -o $O -w $W -l $L
