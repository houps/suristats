CC=gcc
CFLAGS=-W -Wall -Wextra
LDFLAGS=-l sqlite3
EXEC=suristats
SRCDIR=src
OBJDIR=obj
BINDIR=bin
SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(EXEC)

suristats: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)


main.o: $(INC)/dbmgr.h $(INC)/counter.h $(INC)/run.h $(INC)/thread.h

dbmgr.o: $(INC)/counter.h $(INC)/run.h $(INC)/thread.h

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
