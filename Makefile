IDIR=include
SDIR=src
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=$(SDIR)/obj
LDIR =lib

LIBS=-lm

_DEPS = char_vector.h commands.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = char_vector.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_SERVER = server.o commands.o $(_OBJ)

SERVER = $(patsubst %,$(ODIR)/%,$(_SERVER))

_CLIENT = client.o $(_OBJ)

CLIENT = $(patsubst %,$(ODIR)/%,$(_CLIENT))

all: client server

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -g -o $@ $< $(CFLAGS)

client: $(CLIENT)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

server: $(SERVER)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ client server
