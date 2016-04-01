#include <cscript.h>
#include <stdlib.h>
#include <stdio.h>

extern int ush_argv(int, const char * const *av);

static const char *program_path;
static const char *program_name;

int
main(int argc, const char * const *argv)
{
    int rv;

    program_path = *argv;
    program_name = sname(program_path);
    rv = ush_argv(argc, argv);
    dbg_printf("main: rv=%d\n", rv);
    return (rv);
    exit(rv);
}
