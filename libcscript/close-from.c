/*
 * Filename: close-from.c
 * Library: libcscript
 * Brief: Close all file descriptors >= a given number
 *
 * Copyright (C) 2016 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <sys/resource.h>

#include <cscript.h>

/**
 * @brief Determine if a string is numeric
 *
 * More precisely, determine if a string consists of nothing but
 * decimal digits.
 *
 * @param str  IN  The string to be tested.
 * @return true or false
 *
 * There is nothing fancy here -- no radix options, no sign,
 * no leading or trailing space or punctuation -- just ASCII
 * decimal digits.
 *
 */
static bool
isnumeric(const char *str)
{
    const char *s = str;
    while (*s >= '0' && *s <= '9') {
        ++s;
    }
    return (*s == '\0' && s > str);
}

/**
 *
 * @brief Close all file descriptors >= a given number.  Brute force method.
 *
 * @param fd_lo IN  Close file descriptors starting here
 * @return errno-style status
 *
 * Cant use /proc/self/fd
 * We could not open the /proc file descriptor directory.
 * We have to do it the hard way.
 *
 * Scan through all possible file descriptors, up to the resource limit.
 * Close all file descriptors >= fd_lo.
 *
 * Return:
 *     == 0,  if all close() system calls were successful.
 *     != 0,  if any call to close() failed, or if getrlimit() failed.
 *
 *
 * Stop on the first close() that fails.
 *
 */

int
close_from_all(int fd_lo)
{
    int max_fds;
    struct rlimit rl;
    int fd;
    int rc = 0;
    int rv;

    rv = getrlimit(RLIMIT_NOFILE, &rl);
    if (rv != 0) {
        return (errno);
    }

    max_fds = (rl.rlim_max == RLIM_INFINITY) ? INT_MAX : rl.rlim_max;

    for (fd = fd_lo; fd < max_fds; ++fd) {
        int rv;

        if (fcntl(fd, F_GETFD) < 0)
            continue;
        rv = close(fd);
        if (rv != 0 && errno != EBADF) {
            rc = errno;
            break;
        }
    }

    return (rc);
}


/**
 *
 * @brief Close all file descriptors >= a given number.  Visit /proc/self/fd.
 * @param dirp  IN a directory stream that was opened to /proc/self/fd
 * @param fd_lo IN a lower bound file descriptior
 * @return errno-style status
 *
 * Given 1) a directory stream that was opened to /proc/self/fd, and
 *       2) a lower bound file descriptior,
 *
 * Iterate over all numeric files in the directory stream,
 * and close all that are >= the lower bound.
 *
 * Skip closing the fd for the direcory stream, itself.
 * That will be closed by the time we are done.
 *
 * Return:
 *   == 0,  if all calls to close were successful.
 *   != 0,  the errno from the first close() that failed.
 *
 * Stop on the first close() that fails.
 *
 */
int
close_from_dirp(DIR * dirp, int fd_lo)
{
    int rc = 0;
    int rv = 0;
    struct dirent *dp;
    int pfd;

    /*
     * Collect all of the open file descriptors and close
     * the directory before calling 'func' on any of them.
     */
    while ((dp = readdir(dirp)) != NULL) {
        /* skip '.', '..' and the opendir() fd */
        if (!isnumeric(dp->d_name))
            continue;
        pfd = atoi(dp->d_name);
        if (pfd == dirfd(dirp))
            continue;
        if (pfd < fd_lo)
            continue;
        // XXX eprintf("close(%d)\n", pfd);
        rv = close(pfd);
        if (rv != 0 && errno != EBADF) {
            rc = errno;
            break;
        }
    }
    (void)closedir(dirp);
    return (rc);
}

/**
 *
 * @brief Close all open file descriptors >= |fd_lo|.
 *
 * @param fd_lo IN  Close file descriptors starting here
 * @return errno-style status
 *
 * Visits /proc/self/fd to get the set of file descriptors
 * that are actually open, rather than having to try
 * all possible file descriptors.  But, if there is a problem
 * with that, it falls back on the brute force method.
 *
 * Stop on the first close() that fails.
 *
 */
int
close_from(int fd_lo)
{
    int rv;
    DIR *dirp;

    /*
     * Close fd_lo right away as a hedge against failing
     * to open the /proc file descriptor directory due
     * all file descriptors being currently used up.
     */
    rv = close(fd_lo++);
    if (rv != 0 && errno != EBADF) {
        return (errno);
    }

    dirp = opendir("/proc/self/fd");

#ifdef FORCE_CLOSE_FROM_ALL
    // For testing purposes, pretend that opendir('/proc/self/fd') failed.
    // Ensure that close_from_all() get test covereage.
    //
    // XXX Since close_from_all() is exposed (not static)
    // XXX It might be better to write a test driver that calls
    // XXX it, directly, rather than have any variant in code generated,
    // XXX using C preprocessor symbols.

    closedir(dirp);
    dirp = NULL;
#endif

    if (dirp != NULL) {
        rv = close_from_dirp(dirp, fd_lo);
    }
    else {
        rv = close_from_all(fd_lo);
    }

    errno = rv;
    return (rv);
}
