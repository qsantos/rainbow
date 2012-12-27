CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -D_XOPEN_SOURCE=600
LDFLAGS = -O3
OBJDIR  = ../obj
BINDIR  = ../bin
TARGETS = rtgen rtcrack

all: $(addprefix $(BINDIR)/, $(TARGETS))

$(BINDIR)/%: $(addprefix $(OBJDIR)/, %.o md5.o rainbow.o)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -Rf $(OBJDIR)

destroy: clean
	rm -Rf $(BINDIR)

rebuild: destroy all
