## Makefile ##################### S/MIME Gate Project #######

## Macros ###################################################

SOURCES = $(shell find src/ -name '*.c')
OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

CC = gcc
CFLAGS = -pedantic-errors -Wall -Wextra
INCLUDE = include
SRC = src

## Targets ##################################################

all: smime-gate

debug: CFLAGS += -DDEBUG
debug: all

smime-gate: $(OBJECTS)
	$(CC) $^ -o $@

$(SRC)/%.o: $(SRC)/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

test: smime-gate
	cd ./testing; $(MAKE) all

include Makefile.dep

dep:
	@echo Generating dependencies...
	@makedepend -f- -Yinclude -- $(CFLAGS) -- src/*.c \
	    2>/dev/null > Makefile.dep

clean:
	rm -f $(OBJECTS)
	cd ./testing; $(MAKE) clean

.PHONY: all clean dep test

#############################################################

