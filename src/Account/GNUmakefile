all a: Odin-Account

COMMON_OBJ = ../core/core.o
COMMON_H = ../core/core.h

Odin-Account: account.o $(COMMON_OBJ)
	$(CC) -o ../$@ $^
account.o: account.c

clean:
	rm -f *.o
