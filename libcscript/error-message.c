/*
 * Filename: error-message.c
 * Library: libcscript
 * Brief: Support for making an error message stand out with boxes and/or color
 *
 * Description:
 *   General-purpose support for making an error message standout
 *   using a combination of line drawing horizontal rule, or boxes,
 *   or using color.
 *   TUI only.  That is, this is applicable only to plain text terminals.
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

#include <stdio.h>

extern FILE *errprint_fh;


/**
 * @brief Write a horizontal rule, of length |cols|, to the given FILE.
 * @param f    IN Write to this stdio file handle
 * @param cols IN Write a rule this long.
 * @return void
 *
 */
void
fhrule_cols(FILE *f, size_t cols)
{
    while (cols > 0) {
        fputc('-', f);
        --cols;
    }
    fputc('\n', f);
}

/**
 * @brief Write a horizontal rule to the given stdio file handle
 * @return void
 *
 * For now, the length of the rule is 80.
 * It may change to pay attention t whether the file handle is
 * associated with a terminal.
 *
 * It may change to pay attention to the environment variable,
 * COLUMNS, or to some other environment variable,
 * or global value, or option.
 *
 */
void
fhrule(FILE *f)
{
    fhrule_cols(f, 80);
}

/**
 * @brief Write the prologue to an error message.
 * @param f  IN  Write to this stdio file handle.
 * @return void
 *
 * Make an error message to follow stand out with boxes and/or color.
 *
 */
void
ferror_msg_start(FILE *f)
{
    fputs("\e[01;31m\e[K", f);
    fhrule(f);
}

/**
 * @brief Write the epilogue to an error message.
 * @param f  IN  Write to this stdio file handle.
 * @return void
 *
 * This is the complement to ferror_msg_start().
 * An error message has just been written to FILE |f|.
 * Make it stand out with boxes and/or color.
 * Do what is needed (if anything) to return color to "normal".
 *
 */
void
ferror_msg_finish(FILE *f)
{
    fhrule(f);
    fputs("\e[m\e[K", f);
    fflush(f);
}

/**
 * @brief Write the prologue to an error message to |errprint_fh|.
 * @return void
 *
 */
void
error_msg_start(void)
{
    ferror_msg_start(errprint_fh);
}

/**
 * @brief Write the epilogue to an error message to |errprint_fh|.
 *
 */
void
error_msg_finish(void)
{
    ferror_msg_finish(errprint_fh);
}
