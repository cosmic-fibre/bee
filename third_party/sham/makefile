
# sham makefile
#

CC ?= cc

ifdef ENABLE_COVERAGE
CFLAGS_COVERAGE = --coverage
else
CFLAGS_COVERAGE =
endif

CFLAGS_DEBUG   = -DSR_INJECTION_ENABLE -g
CFLAGS_OPT     = -O2
CFLAGS_STRICT  = -std=c99 -pedantic -Wextra -Wall
CFLAGS_MISC    = -Wno-unused-function -fPIC -fno-stack-protector -fvisibility=hidden
SHAM_CFLAGS  = $(CFLAGS_DEBUG) \
                 $(CFLAGS_OPT) \
                 $(CFLAGS_STRICT) \
                 $(CFLAGS_COVERAGE) \
                 $(CFLAGS_MISC) \
                 $(CFLAGS)
SHAM_LDFLAGS = -shared $(LDFLAGS)

all: static dynamic
banner:
	@echo SHAM v1.2.3
	@echo "cc: $(CC)"
	@echo "cflags: $(CFLAGS_DEBUG) $(CFLAGS_COVERAGE)$(CFLAGS_OPT) $(CFLAGS_STRICT)"
	@echo
sham.c:
	@echo build
	@sh sham/build sham $@
	@echo cc $@
sham.h:
	@cp sham/sham/sham.h .
sham.o: banner clean sham.c sham.h
	@$(CC) $(SHAM_CFLAGS) -c sham.c -o sham.o
libsham.a: sham.o
	@echo "ar libsham.a"
	@ar crs libsham.a sham.o
libsham.so: sham.o
	@echo "ld libsham.so"
	@$(CC) sham.o $(SHAM_LDFLAGS) -o libsham.so.1.2.3
	@ln -sf libsham.so.1.2.3 libsham.so.1.2
	@ln -sf libsham.so.1.2.3 libsham.so
	@strip --strip-unneeded libsham.so.1.2.3
static: libsham.a
dynamic: libsham.so
clean:
	@rm -f sham.c sham.h sham.o
	@rm -f libsham.a
	@rm -f libsham.so
	@rm -f libsham.so.1.2
	@rm -f libsham.so.1.2.3

.PHONY: all banner static dynamic clean
