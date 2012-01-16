## Makefile ##################### S/MIME Gate Project #######

## Macros ###################################################

OBJECTS = \
	src/config.o src/error.o src/main.o src/rwwrap.o \
	src/signal.o src/smime-gate.o src/smime.o \
	src/smtp-lib.o src/smtp-types.o src/smtp.o \
	src/sysenv.o src/wrapsock.o src/wrapunix.o

CC = gcc
CFLAGS = -pedantic-errors -Wall -Wextra
INCLUDE = include
SRC = src

## Targets ##################################################

all: smime-gate

smime-gate: $(OBJECTS)
	$(CC) $^ -o $@

$(SRC)/%.o: $(SRC)/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

include Makefile.dep

dep:
	makedepend -f- -Yinclude -- $(CFLAGS) -- src/*.c \
	    2>/dev/null > Makefile.dep

clean :
	    rm -f $(OBJECTS)

.PHONY : all clean dep

#############################################################

