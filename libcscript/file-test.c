/*
 * Filename: file-test.c
 * Library: libcscript
 * Brief: test properties of a file, using single letter tests a la shell/perl.
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

#include <errno.h>
    // Import var errno
#include <fcntl.h>
    // Import faccessat()
    // Import fstat()
    // Import lstat()
#include <stdbool.h>
    // Import constant true
    // Import type bool
#include <string.h>
    // Import strcmp()
#include <sys/stat.h>
    // Import fstat()
    // Import lstat()
#include <sys/types.h>
    // Import fstat()
    // Import geteuid()
    // Import getuid()
    // Import lstat()
#include <unistd.h>
    // Import faccessat()
    // Import fstat()
    // Import geteuid()
    // Import getuid()
    // Import lstat()


/**
 * @brief Apply a single letter file test to a file name.
 * @param fname  IN  A pointer to a struct stat.
 * @param tchr   IN  A single-letter test, a la shell/perl file tests.
 * @return An errno-like status.
 *
 * Most tests are handled by the function, file_test_stat(),
 * which takes a pointer to a |struct stat|, so many tests
 * can be performed without repeating a filename lookup.
 * But, test requiring access(), { r w x R W X }, cannot use a |struct stat|,
 * so we handle those tests here.
 * 
 */
int
file_test_access(const char *fname, int tchr)
{
    int acc;
    int flags;

    switch (tchr) {
        // Effective UID access
        //
        case 'r': acc = R_OK; flags = AT_EACCESS; break;
        case 'w': acc = W_OK; flags = AT_EACCESS; break;
        case 'x': acc = X_OK; flags = AT_EACCESS; break;

        // Real UID access
        //
        case 'R': acc = R_OK; flags = 0; break;
        case 'W': acc = W_OK; flags = 0; break;
        case 'X': acc = X_OK; flags = 0; break;
        default:
            return (EINVAL);
    }
    int rv = faccessat(AT_FDCWD, fname, acc, flags);
    if (rv != 0) {
        return (errno);
    }
    return (0);
}

/**
 * @brief Apply a single letter file test to a struct stat.
 * @param statp  IN  A pointer to a struct stat.
 * @param tchr   IN  A single-letter test, a la shell/perl file tests.
 * @return An errno-like status.
 *
 */
int
file_test_stat(struct stat *statp, int tchr)
{
    int ftype;
    mode_t mode;
    bool tv;

    mode  = statp->st_mode;
    ftype = mode & S_IFMT;

    switch (tchr) {
        // File types
        //
        case 'e': return (0);
        case 'f': tv = (ftype == S_IFREG);  break;
        case 'd': tv = (ftype == S_IFDIR);  break;
        case 'b': tv = (ftype == S_IFBLK);  break;
        case 'c': tv = (ftype == S_IFCHR);  break;
        case 'p': tv = (ftype == S_IFIFO);  break;
        case 'l': tv = (ftype == S_IFLNK);  break;
        case 'S': tv = (ftype == S_IFSOCK); break;

        // File size predicates
        //
        case 'z': tv = (statp->st_size == 0); break;
        case 's': tv = (statp->st_size != 0); break;

        // Other mode bits
        //
        case 'u': tv = ((mode & S_ISUID) != 0); break;
        case 'g': tv = ((mode & S_ISGID) != 0); break;
        case 'k': tv = ((mode & S_ISVTX) != 0); break;


        // Ownership
        //
        case 'o':
            tv = (statp->st_uid == geteuid()); break;
        case 'O':
            tv = (statp->st_uid == getuid()); break;
        default:
            return (EINVAL);
    }

    return (tv ? 0 : -1);
}

/**
 * @brief Do one or more file tests.  Test are single letters, a la shell/perl.
 * @param tests  IN  A string of letters, each letter is a shell-like test.
 * @param fname  IN  The file to be tested.
 * @return errno-like status.
 *
 * 0 for no error,
 * non-zero is an error, typically an errno value or -1.
 *
 * The meaning of the letters in |tests| should all be very stright-forward.
 * They are all just like the shell or perl file test operators.
 *
 * There is one exception:  If the first letter in |tests| is 'L',
 * then the file tests are applied using lstat(), so if |fname|
 * is a symbolic link, all the test apply to the link itself,
 * rather than to the file to which the link resolves.
 *
 */
int
file_test(const char *tests, const char *fname)
{
    struct stat statb;
    const char *t;
    int rv;
    int err;

    t = tests;
    if (strcmp(fname, "-") == 0) {
        rv = fstat(0, &statb);
    }
    else {
        if (*t == 'L') {
            ++t;
            rv = lstat(fname, &statb);
        }
        else {
            rv = stat(fname, &statb);
        }
    }

    if (rv != 0) {
        err = errno;
        return (err);
    }

    for ( ; *t; ++t) {
        int tchr = *t;
        switch (tchr) {
            // Access tests
            //
            case 'r': case 'w': case 'x':
            case 'R': case 'W': case 'X':
                rv = file_test_access(fname, *t);
                break;
            default:
                rv = file_test_stat(&statb, *t);
        }
        if (rv != 0) {
            return (rv);
        }
    }

    // All test passed.
    //
    return (0);
}
