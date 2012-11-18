CFLAGS  = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=700
LDFLAGS = -O3 -lpthread
TARGET  = server
SRC     = main.c socket.c md5.o
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
