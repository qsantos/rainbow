CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=600
LDFLAGS = -O3
TARGETS = rtgen rtcrack

all: $(TARGETS)

rtgen: rtgen.o md5.o rainbow.o
	$(CC) $(LDFLAGS) $^ -o $@

rtcrack: rtcrack.o md5.o rainbow.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy all
