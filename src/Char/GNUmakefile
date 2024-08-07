all c: Odin-Character

COMMON_OBJ = ../core/core.o
COMMON_H = ../core/core.h

Odin-Character: char.o $(COMMON_OBJ)
	$(CC) -o ../$@ char.o $(COMMON_OBJ)
char.o: char.c

clean:
	rm -f *.o
