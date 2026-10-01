/**
 * @file common.h
 * @author kernkraftwerk (kernkraftdev@hotmail.com)
 * @brief common libraries, portale to BSD's, can also compile, maybe on macos
 * @version 0.1
 * @date 2026-09-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef KK_COMMON_H
#define KK_COMMON_H

/* sys/types.h first: some BSD headers rely on it being included before them */
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <semaphore.h>
#include <semaphore.h> // for sem_t
#include <signal.h>
#include <signal.h> // for sig_atomic_t, siginfo_t
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
/*  Platform detection */
#if defined(__linux__)
#define COMMON_LINUX 1
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#define COMMON_BSD 1
#endif

/*  Compiler helpers */
#if defined(__GNUC__) || defined(__clang__)
#define KK_NORETURN __attribute__((noreturn))
#else
#define KK_NORETURN
#endif

/* Small compat shims  */
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON /* BSD spelling */
#endif
#if !defined(MAP_ANON) && defined(MAP_ANONYMOUS)
#define MAP_ANON MAP_ANONYMOUS /* Linux spelling */
#endif

/* Error helpers  */
static inline KK_NORETURN void
error_exit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static inline void
error_exit_failure(int val, const char *msg)
{
	if (val == -1)
		error_exit(msg);
}

#endif /* KK_COMMON_H */