CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=600
LDFLAGS = -O3
TARGETS = rtgen rtcrack

all: $(TARGETS)

rt%: rt%.o md5.o rainbow.o
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
