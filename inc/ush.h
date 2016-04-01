#ifndef _USH_H
#define _USH_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/types.h>

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

struct cmd {
    int argc;
    char **argv;
    const char *cmd_path;
    const char *cmd_name;
    bool cmd_fork;

    // Options
    bool verbose;
    bool debug;

    // Actions -- after fork(), if any, and before exec()
    const char *child_stdin;
    const char *child_stdout;
    bool  child_stdout_new;
    const char *child_stderr;
    bool  child_stderr_new;
    int   ioerr;
    bool  surprise;

    // State
    pid_t child;
    int child_status;
    int rc;
};

typedef struct cmd cmd_t;

enum encoding {
    ENC_INVALID,
    ENC_TEXT,
    ENC_NULL,
    ENC_QP,
    ENC_XNN,
};

typedef enum encoding encoding_t;

// After fork(), if any, and before exec()
//
extern void cmd_chdir(cmd_t *, const char *dir);
extern void cmd_umask(cmd_t *, const char *mask);
extern int set_stdin (cmd_t *, const char *fname);
extern int set_stdout(cmd_t *, const char *fname);
extern int set_stderr(cmd_t *, const char *fname);
extern int ush_close_from(const char *start_fd);

extern int run_program(cmd_t *);
extern int run_interpret_xfname(cmd_t *, char *xfname);
// extern int run_interpret_stream(cmd_t *, FILE *, char *xfname);
extern int ush_argv(int argc, char **argv);


extern void lsdlh(const char *fname);

#ifdef  __cplusplus
}
#endif

#endif /* _USH_H */
