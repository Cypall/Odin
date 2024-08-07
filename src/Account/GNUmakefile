all a: Odin-Account

COMMON_OBJ = ../Core/core.o ../Core/socket.o ../Core/timer.o
COMMON_H = ../Core/core.h ../Core/socket.h ../Core/timer.h ../Core/common.h

Odin-Account: account.o $(COMMON_OBJ)
	$(CC) -o ../$@ $^

account.o: account.c

clean:
	rm -f *.o
