#include <stdlib.h>
#include <stdio.h>

extern int ush(int, char **);
extern char *sname(const char *);
extern void set_print_fh(void);

static char *program_path;
static char *program_name;

int
main(int argc, char **argv)
{
    int rv;

    set_print_fh();
    program_path = *argv;
    program_name = sname(program_path);
    char *cmdv[] = {
        "--verbose",
        "--stdout", "tmp-date-out",
        "--command",
        "/bin/date",
        NULL
        };

    // XXX Use ELEMENTS_OF
    rv = ush(5, cmdv);
fprintf(stderr, "main: rv=%d\n", rv);
    return (rv);
    exit(rv);
}
