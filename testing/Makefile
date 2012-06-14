## Makefile ############# S/MIME Gate Project Testing #######

## Macros ###################################################

SMIME_CLI = smime-gate-test/client.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/signal.o \
	../src/smtp-lib.o ../src/rwwrap.o ../src/error.o \
	../src/smtp.o ../src/smtp-types.o
SMIME_SRV = smime-gate-test/server.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/signal.o \
	../src/smtp-lib.o ../src/rwwrap.o ../src/error.o \
	../src/smtp.o ../src/smtp-types.o

SMTP_1_CLI = smtp-test-1/client.o \
	../src/wrapsock.o ../src/smtp-lib.o ../src/rwwrap.o \
	../src/error.o
SMTP_1_SRV = smtp-test-1/server.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/smtp-lib.o \
       	../src/rwwrap.o ../src/error.o ../src/smtp.o \
	../src/smtp-types.o

SMTP_2_CLI = smtp-test-2/client.o \
	../src/wrapsock.o ../src/smtp-lib.o ../src/rwwrap.o \
	../src/error.o
SMTP_2_SRV = smtp-test-2/server.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/smtp-lib.o \
       	../src/rwwrap.o ../src/error.o

SMTP_3_CLI = smtp-test-3/client.o \
	../src/wrapsock.o ../src/smtp-lib.o ../src/rwwrap.o \
	../src/error.o ../src/smtp.o ../src/smtp-types.o
SMTP_3_SRV = smtp-test-3/server.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/smtp-lib.o \
       	../src/rwwrap.o ../src/error.o ../src/smtp.o \
	../src/smtp-types.o

SMTP_4_CLI = smtp-test-4/client.o \
	../src/wrapsock.o ../src/smtp-lib.o ../src/rwwrap.o \
	../src/error.o
SMTP_4_SRV = smtp-test-4/server.o \
	../src/wrapunix.o ../src/wrapsock.o ../src/smtp-lib.o \
       	../src/rwwrap.o ../src/error.o ../src/smtp.o \
	../src/smtp-types.o

CC = gcc
CFLAGS = -pedantic -Wall -Wextra
INCLUDE = ../include

## Targets ##################################################

all: smime-gate-test smtp-test-1 smtp-test-2 smtp-test-3 smtp-test-4

smime-gate-test: $(SMIME_CLI) $(SMIME_SRV)
	$(CC) $(SMIME_CLI) -o smime-gate-test/client
	$(CC) $(SMIME_SRV) -o smime-gate-test/server

smtp-test-1: $(SMTP_1_CLI) $(SMTP_1_SRV)
	$(CC) $(SMTP_1_CLI) -o smtp-test-1/client
	$(CC) $(SMTP_1_SRV) -o smtp-test-1/server

smtp-test-2: $(SMTP_2_CLI) $(SMTP_2_SRV)
	$(CC) $(SMTP_2_CLI) -o smtp-test-2/client
	$(CC) $(SMTP_2_SRV) -o smtp-test-2/server

smtp-test-3: $(SMTP_3_CLI) $(SMTP_3_SRV)
	$(CC) $(SMTP_3_CLI) -o smtp-test-3/client
	$(CC) $(SMTP_3_SRV) -o smtp-test-3/server

smtp-test-4: $(SMTP_4_CLI) $(SMTP_4_SRV)
	$(CC) $(SMTP_4_CLI) -o smtp-test-4/client
	$(CC) $(SMTP_4_SRV) -o smtp-test-4/server


smime-gate-test/%.o: smime-gate-test/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

smtp-test-1/%.o: smtp-test-1/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

smtp-test-2/%.o: smtp-test-2/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

smtp-test-3/%.o: smtp-test-3/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@

smtp-test-4/%.o: smtp-test-4/%.c Makefile
	$(CC) -c -I$(INCLUDE) $(CFLAGS) $< -o $@


include Makefile.dep

dep:
	makedepend -f- -Y../include -- $(CFLAGS) -- \
	    smtp-test-{1,2,3,4}/*.c smime-gate-test/*.c \
	    2>/dev/null > Makefile.dep

clean:
	rm -f smime-gate-test/*.o
	rm -f smtp-test-{1,2,3,4}/*.o
	rm -f smime-gate-test/{server,client}
	rm -f smtp-test-{1,2,3,4}/{server,client}


.PHONY : all clean dep

#############################################################

