SRCS = reverse.c
EPS = $(SRCS:.c=.i)
ASM = $(SRCS:.c=.s)
OBJS = $(SRCS:.c=.o)
TARG = $(SRCS:.c=.out)
TARG = $(SRCS:.c=)

CC = gcc
OPTS = -g -Wall -Werror -std=c99 -O0
LIBS =

all : $(TARG)

% : %.o
	$(CC) $(LIBS) $< -o $@

%.o : %.c
	$(CC) $(OPTS) -c $< -o $@

%.i : %.c
	$(CC) -E $< -o $@

clean:
	rm -f $(OBJS) $(TARG) $(EPS) $(ASM) $(BIN)

obj: $(OBJS)

asm:
	$(CC) $(OPTS) -S $(SRCS)

eps: $(EPS)
