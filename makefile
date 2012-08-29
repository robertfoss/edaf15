# for power.ludat.lth.se
#CFLAGS	= -m64 -O3 -g -Wall -Wextra -Werror -std=c99 -mcpu=970 -mtune=970 -maltivec

# the following works on any machine
CFLAGS	= -O3 -g -Wall -Wextra -Werror -std=c99 -ftree-vectorize -Wno-unused-parameter -Wno-unused-result -D_POSIX_C_SOURCE=101


CC	= gcc 
FM_OUT	= fm
FM_OBJS	= main.o dt08rf1.o
MATMUL_OUT = matmul
MATMUL_OBJS = matmul.o

all: fm matmul

fm: $(FM_OBJS)
	$(CC) $(CFLAGS) $(FM_OBJS) -o $(FM_OUT)
	size dt08rf1.o

matmul: $(MATMUL_OBJS)
	$(CC) $(CFLAGS) $(MATMUL_OBJS) -o $(MATMUL_OUT)

clean:
	rm -f *.o $(MATMUL_OUT) $(FM_OUT)

.PHONY: all clean
