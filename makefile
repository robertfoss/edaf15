# for power.ludat.lth.se
#CFLAGS	= -m64 -O3 -g -Wall -Wextra -Werror -std=c99 -mcpu=970 -mtune=970 -maltivec

# the following works on any machine
CFLAGS	= -O3 -g -Wall -Wextra -Werror -std=c99 -ftree-vectorize


CC	= gcc 
FM_OUT	= fm
FM_OBJS	= main.o name_fm.o
MATMUL_OUT = matmul
MATMUL_OBJS = matmul.o

all: fm matmul

fm: $(FM_OBJS)
	$(CC) $(CFLAGS) $(FM_OBJS) -o $(FM_OUT)
	size name_fm.o

matmul: $(MATMUL_OBJS)
	$(CC) $(CFLAGS) $(MATMUL_OBJS) -o $(MATMUL_OUT)

clean:
	rm -f *.o $(MATMUL_OUT) $(FM_OUT)

.PHONY: all clean
