C=gcc
FLAGS=-O2 -Wall -Werror
SOURCE=./src
INCLUDE=./include
BIN=./bin
OBJ=main.o bitio.o header.o treemanager.o lz77comp.o lz77dec.o
OUT=lz77	

all: 	$(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) -lm
	rm *.o
	cp $(OUT) $(BIN)

main.o:	
	$(CC) $(FLAGS) -c $(SOURCE)/main.c

bitio.o: $(INCLUDE)/bitio.h
	$(CC) $(FLAGS) -c $(SOURCE)/bitio.c 	

header.o: $(INCLUDE)/header.h $(INCLUDE)/bitio.h
	$(CC) $(FLAGS) -c $(SOURCE)/header.c  

treemanager.o: $(INCLUDE)/treemanager.h
	$(CC) $(FLAGS) -c $(SOURCE)/treemanager.c

lz77comp.o: $(INCLUDE)/lz77.h $(INCLUDE)/header.h $(INCLUDE)/treemanager.h
	$(CC) $(FLAGS) -c $(SOURCE)/lz77comp.c

lz77dec.o: $(INCLUDE)/lz77.h $(INCLUDE)/header.h
	$(CC) $(FLAGS) -c $(SOURCE)/lz77dec.c

clean:
	rm $(OUT)
	rm $(BIN)/$(OUT)

clean_obj:
	rm *.o
	
