CFLAGS  = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=600
LDFLAGS = -O3
TARGET  = rainbow
SRC     = main.c md5.c rainbow.c
OBJ     = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	gcc $(LDFLAGS) $^ -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGET)

rebuild: destroy all
