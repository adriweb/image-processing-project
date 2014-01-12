CC = gcc
CFLAGS = -O2 -W
LDFLAGS = 
 
SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)
AOUT = projet_gcc
 
all : $(AOUT) 
 
projet_gcc : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< -lm
	
clean :
	rm -f *.o $(AOUT)
