OBJS    = main.o functions.o
SOURCE  = main.c functions.c
HEAD	= functions.h
OUT     = app
CC      = gcc
FLAGS   = -c -ggdb3 -w

all:	$(OBJS)
		$(CC) -g $(OBJS) -o $(OUT)

main.o:	main.c
		$(CC) $(FLAGS) main.c

functions.o: functions.c
		$(CC) $(FLAGS) functions.c
clean:
		rm -f $(OBJS) $(OUT)
