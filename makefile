all: CChat

.PHONY: all
.PHONY: clean

server.o:server.c CChat.h list.h
	cc -c server.c

client.o:client.c CChat.h
	cc -c client.c

common.o:common.c CChat.h
	cc -c common.c

tools.o:tools.c CChat.h
	cc -c tools.c

list.o:list.c list.h
	cc -c list.c

CChat: CChat.c server.o client.o common.o tools.o list.o CChat.h
	cc CChat.c server.o client.o common.o tools.o list.o -o CChat -lcurses -pthread

clean: 
	rm -f *.o CChat


