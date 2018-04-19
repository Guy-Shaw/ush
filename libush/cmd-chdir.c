/*
 * Filename: cmd-chdir.c
 * Library: libush
 * Brief: Change directory before running the child process
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

#include <libexplain/chdir.h>

/**
 * @brief Command-line option to Change directory before running the program
 *
 * @param cmd  IN  Command "object" that hold context/control information
 * @param dir  IN  The directory to chdir() to.
 * @return NONE
 *
 * Status is not returned.  Rather, the status is recorded in |cmd|.
 *
 */
int
cmd_chdir(cmd_t *cmd, const char *dir)
{
    int rv;
    int err;

    rv = chdir(dir);
    if (rv == 0) {
        return (rv);
    }

    err = errno;
    eprintf("chdir('");
    fshow_fname(errprint_fh, dir);
    fshow_errno(errprint_fh, "') failed; ", err);

    if (rv != 0) {
        char msg[3000];

        explain_message_errno_chdir(msg, sizeof(msg), err, dir);
        explain_fmt_fopen(msg);
        if (err != ENOENT) {
            // XXX Find real offending file, might be higher up.
            lsdlh(dir);
        }
    }

    cmd->ioerr = err;
    return (err);
}
