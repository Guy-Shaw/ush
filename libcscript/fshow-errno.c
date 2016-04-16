/*
 * Filename: fshow-errno.c
 * Library: libcscript
 * Brief: <<brief-description>>
 * Note: Duplication of file, 'explain-err.c'
 * Note: Deprecate fshow_errno(); favor fexplain_err()
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

#include <cscript.h>

#include <string.h>	// Import strerror_r()

void
fshow_errno(FILE *f, char const *msg, int err)
{
    char esymbuf[12];
    char estrbuf[100];
    char *esymp;
    char *estr;

    esymp = decode_esym_r(esymbuf, sizeof (esymbuf), err);
    estr  = strerror_r(err, estrbuf, sizeof (estrbuf));
    fprintf(f, "%s%d=%s=%s.\n", msg, err, esymp, estr);
}
