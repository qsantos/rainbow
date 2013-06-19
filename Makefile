CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -ansi -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=700 -g
LDFLAGS = -O3 -lm -lpthread
TARGETS = rtgen rtcrack

all: $(TARGETS)

rt%: rt%.o md5.o rtable.o
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy all
