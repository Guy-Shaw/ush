/*
 * Filename: run-interpret.c
 * Library: libush
 * Brief: Run the microshell as an interpreter of a shell script
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
#include <cs-strv.h>
#include <unistd.h>
#include <string.h>     // Import strdup()

#include <libexplain/fopen.h>
#include <libexplain/fclose.h>

extern int ush_getopt(cmd_t *cmd, int argc, char **argv, bool setargv);

extern encoding_t script_encoding;

extern bool opt_append_argv;
extern char *replace;

void *
guard_mem(void *obj)
{
    if (obj != NULL) {
        return (obj);
    }

    eprintf("Out of memory.\n");
    exit (2);
}

// ################ linebuf

struct linebuf {
    FILE   *f;
    char   *buf;
    void   *sgl;
    size_t siz;
    size_t len;
    int    err;
    bool   eof;
};

typedef struct linebuf linebuf_t;

extern linebuf_t *linebuf_new(void);
extern void linebuf_init(linebuf_t *lbuf, FILE *f);
extern void linebuf_sgl_new(linebuf_t *lbuf);
extern void linebuf_free(linebuf_t *lbuf);
extern char *sgl_fgetline(linebuf_t *lbuf, int endl);


static char *
get_line_xnn(linebuf_t *lbuf)
{
    extern ssize_t xnn_decode_str(char *buf, size_t sz, const char *str);
    char *rbuf;
    ssize_t qprv;
    int chr;

    rbuf = sgl_fgetline(lbuf, '\n');

    dbg_show_svar("rbuf", rbuf);
    /*
     * It is safe to xnn_decode_str() in place, because the decoded string
     * is the same length or shorter at every stage during decoding.
     */

    // XXX Get away from using just ASCII Zstrings.
    // XXX Use a string descriptor.  That is, a bounded buffer
    // XXX with a { pointer, length } pair and an associated errno, and eof.
    qprv = xnn_decode_str(rbuf, strlen(rbuf), rbuf);
    dbg_show_svar("rbuf", rbuf);
    if (qprv < 0) {
        lbuf->err = -qprv;
        return (NULL);
    }
    return (rbuf);
}

static char *
get_line_qp(linebuf_t *lbuf)
{
    extern ssize_t qp_decode_str(char *buf, size_t sz, const char *str);
    char *rbuf;
    ssize_t qprv;
    int chr;

    rbuf = sgl_fgetline(lbuf, '\n');

    dbg_show_svar("rbuf", rbuf);
    /*
     * It is safe to qp_decode_str() in place, because the decoded string
     * is the same length or shorter at every stage during decoding.
     */

    // XXX Get away from using just ASCII Zstrings.
    // XXX Use a string descriptor.  That is, a bounded buffer
    // XXX with a { pointer, length } pair and an associated errno, and eof.
    qprv = qp_decode_str(rbuf, strlen(rbuf), rbuf);
    dbg_show_svar("rbuf", rbuf);
    if (qprv < 0) {
        lbuf->err = -qprv;
        return (NULL);
    }
    return (rbuf);
}

static void
fgetline(linebuf_t *lbuf)
{
    char *rbuf;
    int err;

    dbg_printf("> %s\n", __FUNCTION__);

    // Free up any resources leftover from the last time
    // this line buffer was used.
    //
    // In particlur, free up any segments of a scatter/gather list
    // and the line buffer.
    //
    linebuf_free(lbuf);

    linebuf_sgl_new(lbuf);

    if (script_encoding == ENC_NULL) {
        dbg_printf("%s --encoding=null\n", __FUNCTION__);
        rbuf = sgl_fgetline(lbuf, '\0');
    }
    else if (script_encoding == ENC_XNN) {
        dbg_printf("%s --encoding=xnn\n", __FUNCTION__);
        rbuf = get_line_xnn(lbuf);
    }
    else if (script_encoding == ENC_QP) {
        dbg_printf("%s --encoding=qp\n", __FUNCTION__);
        rbuf = get_line_qp(lbuf);
    }
    else {
        dbg_printf("%s  --encoding=text\n", __FUNCTION__);
        rbuf = sgl_fgetline(lbuf, '\n');
    }
    dbg_printf("line: [%s]\n", rbuf);
    dbg_printf("len = %zu\n", strlen(rbuf));
}

enum section {
    SECTION_OPTIONS,
    SECTION_CMDV,
    SECTION_EOF,
};

static int
run_interpret_linebuf(cmd_t *cmd, linebuf_t *lbuf)
{
    enum section scn = SECTION_OPTIONS;
    int rv = 0;
    strv_t cmd_strv;
    char **cmd_argv;
    int    cmd_argc;
    bool in_argv;

    while (true) {
        fgetline(lbuf);
        if (lbuf->eof) {
            scn = SECTION_EOF;
            break;
        }
        if (lbuf->len == 0) {
            continue;
        }
        if (lbuf->buf[0] == '#') {
            continue;
        }
        if (strcmp(lbuf->buf, "--") == 0) {
            scn = SECTION_CMDV;
            break;
        }

        if (lbuf->buf[0] == '-') {
            char  *optv[3];
            optv[0] = ":";
            optv[1] = ":";
            optv[2] = NULL;
            ush_getopt(cmd, 2, &optv[0], false);
            dbg_printf("option: [%s]\n", lbuf->buf);
            optv[0] = ":";
            optv[1] = lbuf->buf;
            optv[2] = NULL;
            ush_getopt(cmd, 2, &optv[0], false);
        }
    }

    if (scn == SECTION_EOF) {
        linebuf_free(lbuf);
        return (rv);
    }

    cmd_strv = strv_null;
    cmd_strv.sv_grow = 100;
    cmd_strv.sv_fatal = true;
    cmd_argv = NULL;
    cmd_argc = 0;
    rv = 0;
    in_argv = false;
    while (true) {
        fgetline(lbuf);
        if (lbuf->eof) {
            scn = SECTION_EOF;
            break;
        }

        // Allow leading blank lines and #-comments,
        // but not blank lines or comments interspersed with arguments.
        //
        if (!in_argv) {
            if (lbuf->len == 0) {
                continue;
            }
            if (lbuf->buf[0] == '#') {
                continue;
            }
            in_argv = true;
        }

        if (replace != NULL && strcmp(lbuf->buf, replace) == 0) {
            if (cmd->argc >= 2) {
                strv_alloc(&cmd_strv, cmd->argc - 1);
                cmd_argv = cmd_strv.strv;
                for (size_t i = 1; i < cmd->argc; ++i) {
                    // XXX Later, keep track of which strings are copies
                    // XXX (which must be freed) and which are references.
                    cmd_argv[cmd_argc] = guard_mem(strdup(cmd->argv[i]));
                    ++cmd_argc;
                }
            }
        }
        else {
            strv_alloc(&cmd_strv, 1);
            cmd_argv = cmd_strv.strv;
            cmd_argv[cmd_argc] = guard_mem(strdup(lbuf->buf));
            ++cmd_argc;
        }
    }

    linebuf_free(lbuf);

    /*
     * If --append-argv and there are any arguments
     * then append the arguments given on the command line
     * of the @program{ush} to the end of the argument list
     * of the program we are about to run.
     */
    if (opt_append_argv && cmd->argc >= 2) {
        strv_alloc(&cmd_strv, cmd->argc - 1);
        cmd_argv = cmd_strv.strv;
        for (size_t i = 1; i < cmd->argc; ++i) {
            // XXX Later, keep track of which strings are copies
            // XXX (which must be freed) and which are references.
            cmd_argv[cmd_argc] = guard_mem(strdup(cmd->argv[i]));
            ++cmd_argc;
        }
    }

    if (cmd_argc != 0) {
        strv_alloc(&cmd_strv, 1);
        cmd_argv = cmd_strv.strv;
        cmd_argv[cmd_argc] = NULL;
        if (debug && dbgprint_fh != NULL) {
            fshow_str_array(dbgprint_fh, cmd_argc, cmd_argv);
        }
        cmd->argc = cmd_argc;
        cmd->argv = cmd_argv;
        cmd->cmd_path = cmd_argv[0];
        cmd->cmd_name = sname(cmd->cmd_path);
        rv = run_program(cmd);
        strv_free_strings(&cmd_strv);
        strv_free(&cmd_strv);
    }
    return (rv);
}

static int
run_interpret_stream(cmd_t *cmd, FILE *xf, char *xfname)
{
    linebuf_t *lbuf;
    int rv;

    dbg_printf("run_interpret_stream\n");
    lbuf = linebuf_new();
    linebuf_init(lbuf, xf);
    rv = run_interpret_linebuf(cmd, lbuf);
    linebuf_free(lbuf);
    free(lbuf);
    return (rv);
}

int
run_interpret_xfname(cmd_t *cmd, char *xfname)
{
    FILE *xf;
    int rv, rv2;

    rv = file_test("d", xfname);
    if (rv == 0) {
        eprint("'");
        fshow_fname(errprint_fh, xfname);
        eprint("'");
        eprint(" is a directory.\n");
        error_msg_start();
        lsdlh(xfname);
        error_msg_finish();
        return (EISDIR);
    }

    xf = fopen(xfname, "r");
    if (xf == NULL) {
        char msg[3000];
        int err;

        err = errno;
        explain_message_errno_fopen(msg, sizeof(msg), err, xfname, "r");
        explain_fmt_fopen(msg);
        if (err != ENOENT) {
            // XXX Find real offending file, might be higher up.
            error_msg_start();
            lsdlh(xfname);
            error_msg_finish();
        }
        return (err);
    }

    rv = run_interpret_stream(cmd, xf, xfname);
    rv2 = explain_fclose_on_error(xf);
    if (rv) {
        return (rv);
    }
    return (rv2);
}
