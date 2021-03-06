/*
 * Filename: io-redirect.c
 * Library: libush
 * Brief: Do all redeirection of I/O before running cild process
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

#include <libexplain/open.h>

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
set_write_fd(int fd, cmd_t *cmd, const char *fname, bool append, bool new_file)
{
    int old_fd;
    int new_fd;
    int o_flags;
    int o_mode;

    if (new_file) {
        int rv;

        rv = access(fname, F_OK);
        if (rv == 0) {
            fprintf(stderr, "File, '%s' already exists.\n", fname);
            lsdlh(fname);
            cmd->ioerr = EEXIST;
            return (EEXIST);
        }
    }

    o_flags = O_CREAT|O_WRONLY;
    if (append) {
        o_flags |= O_APPEND;
    }

    o_mode = S_IRUSR|S_IWUSR;
    old_fd = open(fname, o_flags, o_mode);
    if (old_fd == -1) {
        char msg[3000];
        int err;

        err = errno;
        explain_message_errno_open(msg, sizeof(msg), err, fname, o_flags, o_mode);
        explain_fmt_fopen(msg);
        if (err != ENOENT) {
            // XXX Find real offending file, might be higher up.
            lsdlh(fname);
        }
        cmd->ioerr = err;
        return (err);
    }

    new_fd = dup2(old_fd, fd);
    if (new_fd == -1) {
        cmd->ioerr = errno;
        return (errno);
    }
    if (new_fd != fd) {
        cmd->ioerr = 0;
        cmd->surprise = 1;
        return (126);
    }
    close(old_fd);
    return (0);
}

int
set_stdout(cmd_t *cmd, const char *fname, bool append, bool new_file)
{
    int rv;

    cmd->child_stdout        = fname;
    cmd->child_stdout_append = append;
    cmd->child_stdout_new    = new_file;
    rv = set_write_fd(1, cmd, fname, append, new_file);
    return (rv);
}

int
set_stderr(cmd_t *cmd, const char *fname, bool append, bool new_file)
{
    int rv;

    cmd->child_stderr        = fname;
    cmd->child_stderr_append = append;
    cmd->child_stderr_new    = new_file;
    rv = set_write_fd(2, cmd, fname, append, new_file);
    return (rv);
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
