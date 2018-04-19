/*
 * Filename: cmd-env.c
 * Library: libush
 * Brief: set an environment variable before running child process
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

#define _GNU_SOURCE 1

#include <ush.h>
#include <cscript.h>

#include <ctype.h>
    // Import isalnum()
    // Import isalpha()
#include <errno.h>
    // Import var EINVAL
#include <stdbool.h>
    // Import type bool
#include <stdlib.h>
    // Import clearenv()
    // Import putenv()

#define UNUSED(var) (void) var

static bool
is_ident_start(int chr)
{
    return (isalpha(chr) || chr == '_');
}

static bool
is_ident(int chr)
{
    return (isalnum(chr) || chr == '_');
}

int
cmd_clearenv(cmd_t *cmd, const char *arg)
{
    UNUSED(cmd);
    UNUSED(arg);

    return (clearenv());
}

int
cmd_env(cmd_t *cmd, const char *kv_assign)
{
    const char *val;
    int err;

    UNUSED(cmd);

    val = kv_assign;
    err = 0;
    while (*val && *val != '=') {
        if (!is_ident(*val)) {
            err = 1;
        }
        ++val;
    }

    if (err != 0 || !is_ident_start(kv_assign[0])) {
        int l = val - kv_assign;
        eprintf("ush::env: Invalid identifier, '%.*s'\n", l, kv_assign);
        return (EINVAL);
    }

    if (*val != '=') {
        eprintf("ush::env: No value for identifier, '%s'\n", kv_assign);
        return (EINVAL);
    }

    return (putenv((char *) kv_assign));
}
