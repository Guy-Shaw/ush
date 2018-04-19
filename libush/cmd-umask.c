/*
 * Filename: cmd-umask.c
 * Library: libush
 * Brief: Change umask before running child process
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

bool
is_octal(const char *s)
{
    uint_t i;
    int d;

    for (i = 0; (d = s[i]) != 0; ++i) {
        if (!(d >= '0' && d <= '7')) {
            return (false);
        }
    }

    return (i > 0);
}

int
parse_umask(const char *mask_str, mode_t *ret)
{
    mode_t mode_u = 0;
    mode_t mode_g = 0;
    mode_t mode_o = 0;
    mode_t *which;      // Select one of { u, g, o }

    dbg_printf("%s: mask_str='%s'\n", __FUNCTION__, mask_str);

    while (*mask_str) {
        switch (*mask_str) {
            case 'u':
                which = &mode_u;
                break;
            case 'g':
                which = &mode_g;
                break;
            case 'o':
                which = &mode_o;
                break;
            default:
                return (EINVAL);
        }

        if (*which != 0) {
            return (EINVAL);
        }
        ++mask_str;
        if (*mask_str != '=') {
            return (EINVAL);
        }
        ++mask_str;
        while (*mask_str && *mask_str != ',') {
            mode_t setbit;

            switch (*mask_str) {
                case 'r':
                    setbit = 4;
                    break;
                case 'w':
                    setbit = 2;
                    break;
                case 'x':
                    setbit = 1;
                    break;
                default:
                    return (EINVAL);
            }
            if (*which & setbit) {
                return (EINVAL);
            }
            *which |= setbit;
            ++mask_str;
        }
        if (*mask_str == ',') {
            ++mask_str;
        }
    }
    *ret = (mode_u << 6) | (mode_g << 3) | mode_o;
    dbg_printf("%s: umask=%o\n", __FUNCTION__, *ret);
    return (0);
}

int
cmd_umask(cmd_t *cmd, const char *mask_str)
{
    int rv;
    unsigned long int lmask;
    mode_t mask;
    bool have_mask = false;

    if (is_octal(mask_str)) {
        lmask = strtoul(mask_str, NULL, 8);
        if (lmask > 0777) {
            eprintf("Invalid umask, '%s'.\n", mask_str);
            eprintf("umask must be in 0..0777 (octal).\n");
            cmd->ioerr = ERANGE;
            return (ERANGE);
        }
        mask = (mode_t)lmask;
        have_mask = true;
    }

    if (!have_mask) {
        rv = parse_umask(mask_str, &mask);
        if (rv != 0) {
            eprintf("Invalid umask, '%s'.\n", mask_str);
            fshow_errno(errprint_fh, " ", rv);
            cmd->ioerr = rv;
            return (rv);
        }
        have_mask = true;
    }

    rv = umask(mask);
    return (rv);
}
