/*
 * Filename: io-redirect.c
 * Library: libush
 * Brief: Do all redeirection of I/O befor running cild process
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

#include <ush.h>
#include <cscript.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int
set_stdin(cmd_t *cmd, const char *fname)
{
    int old_fd;
    int new_fd;

    cmd->child_stdin = fname;
    old_fd = open(fname, O_RDONLY);
    if (old_fd == -1) {
        cmd->ioerr = errno;
        return (old_fd);
    }
    new_fd = dup2(old_fd, 0);
    if (new_fd == -1) {
        cmd->ioerr = errno;
        return (old_fd);
    }
    if (new_fd != 0) {
        cmd->ioerr = 0;
        cmd->surprise = 1;
        return (-1);
    }
    close(old_fd);
    return (0);
}

int
set_stdout(cmd_t *cmd, const char *fname)
{
    int old_fd;
    int new_fd;
    int o_flags;

    cmd->child_stdout = fname;

    if (cmd->child_stdout_new) {
        int rv;

        rv = access(fname, F_OK);
        if (rv == 0) {
            fprintf(stderr, "File, '%s' already exists.\n", cmd->child_stdout);
            lsdlh(cmd->child_stdout);
            cmd->ioerr = EEXIST;
            return (EEXIST);
        }
    }

    o_flags = O_CREAT|O_WRONLY;
    old_fd = open(fname, o_flags, S_IRUSR|S_IWUSR);
    if (old_fd == -1) {
        cmd->ioerr = errno;
        return (errno);
    }
    new_fd = dup2(old_fd, 1);
    if (new_fd == -1) {
        cmd->ioerr = errno;
        return (errno);
    }
    if (new_fd != 1) {
        cmd->ioerr = 0;
        cmd->surprise = 1;
        return (126);
    }
    close(old_fd);
    return (0);
}

int
set_stderr(cmd_t *cmd, const char *fname)
{
    int old_fd;
    int new_fd;
    int o_flags;

    cmd->child_stderr = fname;

    if (cmd->child_stderr_new) {
        int rv;

        rv = access(fname, F_OK);
        if (rv == 0) {
            fprintf(stderr, "File, '%s' already exists.\n", cmd->child_stderr);
            lsdlh(cmd->child_stderr);
            cmd->ioerr = EEXIST;
            return (EEXIST);
        }
    }

    o_flags = O_CREAT|O_WRONLY;
    old_fd = open(fname, o_flags, S_IRUSR|S_IWUSR);
    if (old_fd == -1) {
        cmd->ioerr = errno;
        return (errno);
    }
    new_fd = dup2(old_fd, 2);
    if (new_fd == -1) {
        cmd->ioerr = errno;
        return (errno);
    }
    if (new_fd != 2) {
        cmd->ioerr = 0;
        cmd->surprise = 1;
        return (126);
    }
    close(old_fd);
    return (0);
}

static bool
isnumeric(const char *str)
{
    const char *s = str;
    while (*s >= '0' && *s <= '9') {
        ++s;
    }
    return (*s == '\0' && s > str);
}

int
ush_close_from(const char *arg)
{
    int start_fd;
    int rv;

    if (!isnumeric(arg)) {
        eprintf("--close-from: %s argument must be numeric.\n", arg);
        return (EDOM);
    }
    start_fd = atoi(arg);
    eprintf("ush_close_from: start_fd=%d\n", start_fd);
    rv = close_from(start_fd);
    if (rv) {
        eprintf("close_from(%d) failed; rv=%d.\n", start_fd, rv);
    }
    return (rv);
}
