/*
 * Filename: run-program.c
 * Library: libush
 * Brief: Run a given program.  Either fork() and exec() or just exec()
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

static int
wait_cmd(cmd_t *cmd)
{
    int status;

    status = 0;
    while (true) {
        wait(&status);
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            cmd->child_status = status;
            if (cmd->verbose) {
                eprintf("status=0x%02x\n", cmd->child_status);
            }
            break;
        }
    }
    return (status);
}

int
exec_program(cmd_t *cmd)
{
    int rv;

    cmd->rc = execvp(cmd->cmd_path, cmd->argv);
    if (cmd->rc != 0) {
        perror("execvp()");
        rv = errno;
    }
    else {
        eprintf("execvp() failed for reasons unknown!\n");
        rv = 126;
    }
    if (cmd->cmd_fork) {
        exit(rv);
    }
    return (rv);
}

int
run_child_program(cmd_t *cmd)
{
    int rv;

    cmd->child = fork();
    if (cmd->child == 0) {
        rv = exec_program(cmd);
    }
    else {
        if (cmd->verbose) {
            eprintf("child pid=%d\n", cmd->child);
        }
        rv = wait_cmd(cmd);
    }

    cmd->rc = rv;
    return (rv);
}

int
run_program(cmd_t * cmd)
{
    int rv;

    if (cmd->cmd_fork) {
        rv = run_child_program(cmd);
    }
    else {
        rv = exec_program(cmd);
        dbg_printf("run_program: rv=%d\n", rv);
    }
    return (rv);
}
