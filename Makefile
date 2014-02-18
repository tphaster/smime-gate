## Makefile ##################### S/MIME Gate Project #######

## Macros ###################################################

SOURCES = $(shell find src/ -name '*.c')
OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

CC = gcc
CFLAGS = -pedantic-errors -Wall -Wextra
INCLUDE = include
SRC = src

INSTALL=install

# Common prefix for installation directories.
prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
docdir=$(prefix)/doc
localstatedir=$(prefix)/var
sysconfdir=$(prefix)/etc


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

install: smime-gate
	@echo Installing smime-gate...
	$(INSTALL) -D -s -m 0755 smime-gate $(DESTDIR)$(bindir)/smime-gate
	$(INSTALL) -D -m 0644 resources/config $(DESTDIR)$(sysconfdir)/smime-gate/config
	$(INSTALL) -D -m 0644 resources/rules $(DESTDIR)$(sysconfdir)/smime-gate/rules
	$(INSTALL) -D -m 0644 AUTHORS $(DESTDIR)$(docdir)/smime-gate/AUTHORS
	$(INSTALL) -D -m 0644 COPYING $(DESTDIR)$(docdir)/smime-gate/COPYING
	$(INSTALL) -D -m 0644 LICENSE $(DESTDIR)$(docdir)/smime-gate/LICENSE
	$(INSTALL) -D -m 0644 README $(DESTDIR)$(docdir)/smime-gate/README
	mkdir -p $(DESTDIR)$(localstatedir)/run/smime-gate/unsent

uninstall:
	@echo Uninstalling smime-gate from system...
	@echo Configuration files are left in $(DESTDIR)$(sysconfdir)/smime-gate
	rm -rf $(DESTDIR)$(bindir)/smime-gate
	rm -rf $(DESTDIR)$(docdir)/smime-gate
	rm -rf $(DESTDIR)$(localstatedir)/run/smime-gate

clean:
	rm -f $(OBJECTS)
	cd ./testing; $(MAKE) clean

.PHONY: all clean dep test install uninstall

#############################################################

