# for hardening toolchain see: https://wiki.gentoo.org/wiki/Hardened/Toolchain

CC       = gcc
BUILDDIR = build
BINDIR   = .

LIBNAME  = libsem.so
LIBSEM   = $(BUILDDIR)/$(LIBNAME)

# sources
LIB_SRCS = $(wildcard lib/*.c)
LIB_OBJS = $(LIB_SRCS:%.c=$(BUILDDIR)/%.o)

GEN_SRCS = src/generator.c
GEN_OBJS = $(GEN_SRCS:%.c=$(BUILDDIR)/%.o)

SUP_SRCS = src/supervisor.c
SUP_OBJS = $(SUP_SRCS:%.c=$(BUILDDIR)/%.o)

ALL_OBJS = $(LIB_OBJS) $(GEN_OBJS) $(SUP_OBJS)
DEPS     = $(ALL_OBJS:.o=.d)

GENERATOR  = $(BINDIR)/generator
SUPERVISOR = $(BINDIR)/supervisor

# include paths (common.h, graph.h, semaphore.h all live directly in lib/)
INCLUDES = -Ilib

defs    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default = -std=c99 -pedantic -g -Wall $(defs)
verbose = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -Wformat=2 -Warray-bounds -Wstack-protector -Wshadow $(defs)

# compile-time hardening
# hopefully -O3 doesnt mess me up like Debian
harden_cc = -O3 -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=3 \
            -fPIE -Wformat -Wformat-security -Werror=format-security

# link-time hardening
harden_ld = -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack

asan = -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-common

library = -pthread -lrt

# pick the base flag set with: make MODE=verbose
MODE ?= default

CFLAGS  = $($(MODE)) $(INCLUDES) -MMD -MP $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)

.PHONY: all verbose asan harden doc clean

all: $(GENERATOR) $(SUPERVISOR)

verbose:
	$(MAKE) MODE=verbose all

asan: clean
	$(MAKE) EXTRA_CFLAGS="$(asan)" EXTRA_LDFLAGS="$(asan)" all

harden: clean
	$(MAKE) EXTRA_CFLAGS="$(harden_cc)" EXTRA_LDFLAGS="$(harden_ld)" all
	@echo "Built with compile-time and link-time hardening."

# executables
$(GENERATOR): $(GEN_OBJS) $(LIBSEM)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $(GEN_OBJS) $(LIBSEM) $(library)

$(SUPERVISOR): $(SUP_OBJS) $(LIBSEM)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $(SUP_OBJS) $(LIBSEM) $(library)

# static library
$(LIBSEM): $(LIB_OBJS)
	@mkdir -p $(@D)
	ar rcs $@ $^

# generic object rule (mirrors the source tree under build/)
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	doxygen Doxyfile

clean:
	rm -rf $(BUILDDIR) $(GENERATOR) $(SUPERVISOR)

# header dependency tracking
-include $(DEPS)