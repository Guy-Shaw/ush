/*
 * Filename: ush.c
 * Library: libush
 * Brief: Ush top level.  Either run a program or interpret a script.
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

#include <stdlib.h>         // Import exit()
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>          // Import isprint()
// #include <sys/wait.h>

#include <ush.h>
#include <cscript.h>

#include <getopt_int.h>

#include <unistd.h>         // Import isatty()

static cmd_t cmdbuf;
static cmd_t *cmd = &cmdbuf;

bool verbose  = false;
bool debug    = false;
bool opt_append_argv = false;

encoding_t script_encoding = ENC_TEXT;

char *replace = NULL;

static bool opt_command   = false;
static bool opt_show_argv = false;

FILE *errprint_fh = NULL;
FILE *dbgprint_fh = NULL;

static char *program_name = "ush";

enum opt {
    OPT_BASE = 0xf000,
    OPT_SHOW_ARGV,
    OPT_APPEND_ARGV,
    OPT_FORK,
    OPT_CHDIR,
    OPT_SET_STDIN,
    OPT_SET_STDOUT,
    OPT_SET_STDOUT_APPEND,
    OPT_SET_STDOUT_NEW,
    OPT_SET_STDERR,
    OPT_SET_STDERR_APPEND,
    OPT_SET_STDERR_NEW,
    OPT_UMASK,
    OPT_CLOSE_FROM,
    OPT_REPLACE,
    OPT_ENCODING,
};

static struct option long_options[] = {
    {"help",              no_argument,       0,  'h'},
    {"version",           no_argument,       0,  'V'},
    {"debug",             no_argument,       0,  'd'},
    {"command",           no_argument,       0,  'c'},
    {"append-argv",       no_argument,       0,  OPT_APPEND_ARGV},
    {"show-argv",         no_argument,       0,  OPT_SHOW_ARGV},
    {"fork",              no_argument,       0,  OPT_FORK},
    {"stdin",             required_argument, 0,  OPT_SET_STDIN},
    {"stdout",            required_argument, 0,  OPT_SET_STDOUT},
    {"stdout-append",     required_argument, 0,  OPT_SET_STDOUT_APPEND},
    {"stdout-new",        required_argument, 0,  OPT_SET_STDOUT_NEW},
    {"stderr",            required_argument, 0,  OPT_SET_STDERR},
    {"stderr-append",     required_argument, 0,  OPT_SET_STDERR_APPEND},
    {"stderr-new",        required_argument, 0,  OPT_SET_STDERR_NEW},
    {"chdir",             required_argument, 0,  OPT_CHDIR},
    {"umask",             required_argument, 0,  OPT_UMASK},
    {"close-from",        required_argument, 0,  OPT_CLOSE_FROM},
    {"replace",           required_argument, 0,  OPT_REPLACE},
    {"encoding",          required_argument, 0,  OPT_ENCODING},
    {0, 0, 0, 0 }
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?      Show this help message and exit\n"
    "  --version         Show ush version information and exit\n"
    "  --verbose|-v      verbose\n"
    "  --debug|-d        debug\n"
    "  --command\n"
    "  --show-argv\n"
    "  --stdin         <filename>\n"
    "  --stdout        <filename>\n"
    "  --stdout-append <filename>\n"
    "  --stdout-new    <filename>\n"
    "  --stderr        <filename>\n"
    "  --stderr-append <filename>\n"
    "  --stderr-new    <filename>\n"
    "  --close-from    <fd>\n"
    "  --chdir         <directory>\n"
    "  --fork\n"
    "  --append-argv\n"
    "  --replace       <string>\n"
    "  --encoding      text|null|qp|xnn\n"
    ;

static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_ush_version(FILE *f)
{
    fputs(version_text, f);
    fputc('\n', f);
    fputs(copyright_text, f);
    fputc('\n', f);
    fputs(license_text, f);
    fputc('\n', f);
}

static void
show_ush_version(void)
{
    fshow_ush_version(stdout);
}

static void
usage(void)
{
    eprintf("usage: %s [ <options> ]\n", program_name);
    eprint(usage_text);
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

static encoding_t
parse_encoding(const char *s)
{
    if (strcmp(s, "text") == 0) {
        return (ENC_TEXT);
    }
    if (strcmp(s, "xnn") == 0) {
        return (ENC_XNN);
    }
    if (strcmp(s, "quoted-printable") == 0) {
        return (ENC_QP);
    }
    if (strcmp(s, "qp") == 0) {
        return (ENC_QP);
    }
    if (strcmp(s, "null") == 0) {
        return (ENC_NULL);
    }
    return (ENC_INVALID);
}

static struct _getopt_data null_getopts_data;

void
getopts_init(struct _getopt_data *ctx)
{
    *ctx = null_getopts_data;
}

/*
 * Arguments can come from the command line of @command{ush} itself,
 * or from the script file, one argument per line.
 *
 * Only in the case of the main ush command-line arguments do we want
 * to set the remaining (non-option) arguments in |cmd|.
 *
 * When reading lines from a ush script file, each argument is decoded
 * by building an argument vector using a dummy program name followed
 * by one argument, an option.
 *
 */
int
ush_getopt(cmd_t *cmd, int argc, char **argv, bool setargv)
{
    struct _getopt_data getopt_ctx;
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    int rv;

    getopts_init(&getopt_ctx);
    option_index = 0;
    err_count = 0;
    optind = 1;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        getopt_ctx.optind = optind;
        getopt_ctx.opterr = opterr;
        optc = cs_getopt_internal_r(argc, argv, "+hVcdv", long_options, &option_index, 0, &getopt_ctx, 0);

        optind = getopt_ctx.optind;
        optarg = getopt_ctx.optarg;
        optopt = getopt_ctx.optopt;

        if (optc == -1) {
            break;
        }

        if (debug) {
            dbg_printf("optc=0x%x", optc);
            if (optc >= 0 && optc <= 127 && isprint(optc)) {
                dbg_printf("='%c'", optc);
            }
            eprintf("\n");
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_ush_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            break;
        case 'v':
            verbose = true;
            break;
        case OPT_ENCODING:
            script_encoding = parse_encoding(optarg);
            break;
        case 'c':
            opt_command = true;
            break;
        case OPT_APPEND_ARGV:
            opt_append_argv = true;
            break;
        case OPT_SHOW_ARGV:
            opt_show_argv = true;
            break;
        case OPT_FORK:
            cmd->cmd_fork = true;
            break;
        case OPT_CHDIR:
            cmd_chdir(cmd, optarg);
            break;
        case OPT_UMASK:
            cmd_umask(cmd, optarg);
            break;
        case OPT_SET_STDIN:
            rv = set_stdin(cmd, optarg);
            break;
        case OPT_SET_STDOUT:
            rv = set_stdout(cmd, optarg, false, false);
            break;
        case OPT_SET_STDOUT_APPEND:
            rv = set_stdout(cmd, optarg, true, false);
            break;
        case OPT_SET_STDOUT_NEW:
            rv = set_stdout(cmd, optarg, false, true);
            break;
        case OPT_SET_STDERR:
            rv = set_stderr(cmd, optarg, false, false);
            break;
        case OPT_SET_STDERR_APPEND:
            rv = set_stderr(cmd, optarg, true, false);
            break;
        case OPT_SET_STDERR_NEW:
            rv = set_stderr(cmd, optarg, false, true);
            break;
        case OPT_CLOSE_FROM:
            rv = ush_close_from(optarg);
            break;
        case OPT_REPLACE:
            replace = optarg;
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, ", program_name);
            if (isalpha(optopt)) {
                eprintf("'%c'\n", optopt);
            }
            else {
                eprintf("%d\n", optopt);
            }
            exit(2);
            break;
        }

        if (rv) {
            ++err_count;
        }
    }

    if (err_count) {
        return (err_count);
    }

    verbose = verbose || debug;
    opt_show_argv = opt_show_argv || verbose;
    cmd->verbose = verbose;
    cmd->debug   = debug;

    if (setargv) {
        cmd->cmd_path = argv[optind];
        cmd->cmd_name = sname(cmd->cmd_path);
        cmd->argc = argc - optind;
        cmd->argv = argv + optind;
    }
    return (0);
}

int
ush_argv(int argc, char **argv)
{
    int rv;
    set_print_fh();

    // Make it easy to set --debug and --verbose options via the environment,
    // So that it is less likely that options for @command{ush} itself are
    // not confused with options to be passed to the program to be executed.
    //
    if (getenv("USH_DEBUG") != NULL) {
        debug = true;
    }
    if (getenv("USH_VERBOSE") != NULL) {
        verbose = true;
    }

    rv = ush_getopt(cmd, argc, argv, true);

    if (rv != 0) {
        usage();
        exit(1);
    }

    if (cmd->argc == 0) {
        eprintf("%s: Must supply at least a command name.\n", program_name);
        usage();
        exit(2);
    }

    if (opt_show_argv) {
        fshow_str_array(stderr, cmd->argc, cmd->argv);
    }

    if (cmd->ioerr) {
        exit(2);
    }

    if (opt_command) {
        cmd->child_status = run_program(cmd);
    }
    else {
        dbg_printf("script=%s\n", cmd->argv[0]);
        cmd->child_status = run_interpret_xfname(cmd, cmd->argv[0]);
    }
    dbg_printf("child status=%d\n", cmd->child_status);
    if (cmd->cmd_fork) {
        return (WEXITSTATUS(cmd->child_status));
    }
    else {
        return (cmd->child_status);
    }
}

/*
 * A call to ush_argv() has a complete argv[] including the
 * program name.  So, it can use getopt logic to parse all options.
 * So, for example, the ush main program can just call ush_argv()
 * with its own (argc, argv) arguments, intact.  The program name
 * will be "ush".
 *
 * A call to ush from some other program does not naturally start
 * with the program name; rather it starts with the name of the
 * program we want to run as a child process.  So, the arguments
 * need to be prepended with a program name, in order to use
 * getopt_long to parse options correctly.
 *
 * There is no need to deep copy.  Only the pointers need be copied.
 */

int
ush(int argc, char **argv)
{
    char **cmd_argv;
    size_t cmd_argv_sz;
    int rv;

    set_print_fh();
    cmd_argv_sz = (argc + 2) * sizeof (char *);
    cmd_argv = guard_malloc(cmd_argv_sz);
    cmd_argv[0] = "ush";
    memcpy(cmd_argv + 1, argv, (argc + 1) * sizeof (char *));
    rv = ush_argv(argc + 1, cmd_argv);
    free(cmd_argv);
    return (rv);
}
