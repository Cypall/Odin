all c: Odin-Character

COMMON_OBJ = ../Core/core.o ../Core/socket.o ../Core/timer.o
COMMON_H = ../Core/core.h ../Core/socket.h ../Core/timer.h ../Core/common.h

Odin-Character: char.o $(COMMON_OBJ)
	$(CC) -o ../$@ char.o $(COMMON_OBJ)
char.o: char.c

clean:
	rm -f *.o
