/*
 * Filename: dbg-show-svar.c
 * Library: libcscript
 * Brief: Pretty-print some data types to support debugging
 *
 * Description:
 *   Provide support functions to safely print some data types,
 *   ensuring that only graphic characters are used to represent
 *   any data type.  Also, pretty-print some more complex objects.
 *
 *   The bulk of the support to print more complex objects can be found
 *   in separate files related to modules for  each "class".
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

#include <stdbool.h>
    // Import type bool
#include <stdio.h>
    // Import fprintf()
    // Import type FILE

extern FILE *dbgprint_fh;
extern bool debug;

extern void fshow_str(FILE *f, const char *str);

/**
 *
 * @brief Print the variable name and value, on the specified stdio stream
 *
 * @param f         IN  Print to this stdio file handle
 * @param var_name  IN  The name  of the variable (identifier)
 * @param var_value IN  The value of the variable (string)
 * @return void
 *
 * The variable name and value are printed to the given stdio file handle,
 * |f|.
 *
 * The representation of |var_value| is printed using all graphic
 * characters.
 *
 */

void
fshow_svar(FILE *f, const char *var_name, const char *var_value)
{
    fprintf(f, "%s=[", var_name);
    fshow_str(f, var_value);
    fprintf(f, "]\n");
}

/**
 *
 * @brief Print the given variable name and value, but only if --debug option
 *
 * @param var_name  IN  The name  of the variable (identifier)
 * @param var_value IN  The value of the variable (string)
 *
 * The variable name and value are printed to the stdio file handle,
 * |dbgprint_fh|.
 *
 * The representation of |var_value| is printed using all graphic
 * characters.
 *
 * If --debug option is not in effect, then do nothing.
 * This is to make it easy to spinkle calls to dbg_show_svar()
 * in various places, without having to worry about testing |debug|.
 *
 */

void
dbg_show_svar(const char *var_name, const char *var_value)
{
    if (debug) {
        fshow_svar(dbgprint_fh, var_name, var_value);
    }
}
