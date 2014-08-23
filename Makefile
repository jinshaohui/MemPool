CC=gcc
CFLAGS=-g -Wall 

mpool:*.h *.c
	${CC} ${CFLAGS} -o $@ $^
