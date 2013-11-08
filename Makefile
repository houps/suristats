CC=gcc
CFLAGS=-W -Wall -Wextra
LDFLAGS=-l sqlite3
TARGET=suristats
SRCDIR=src
OBJDIR=obj
BINDIR=bin
SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

#all: $(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)


obj/main.o: $(SRCDIR)/dbmgr.h $(SRCDIR)/counter.h $(SRCDIR)/run.h $(SRCDIR)/thread.h

obj/dbmgr.o: $(SRCDIR)/counter.h $(SRCDIR)/run.h $(SRCDIR)/thread.h

.PHONY: clean mrproper

clean:
	rm -rf $(OBJECTS)

mrproper: clean
	rm -rf $(TARGET)
