# ush -- micro-shell

## a very light-weight "shell" library and command

The micro-shell, `ush`, is a very tiny shell with extremely simple
syntax.  It is designed to provide just enough functionality
to cover 80-90% of the use cases of `system()` and `exec()`
functions in other languages, but more safely and efficiently.

The `libush` function `ush_argv()` is more or less like
the `exec()` and `system()` functions that exist in C
and in higher-level languages like Perl, Python, Ruby, etc.
The main difference is that `ush` has commands to do things
just before running the command, like
redirect I/O, change directory, change privileges, etc.

The reason this is important is that `system()`, for example in Perl,
pretty much encourages people to run a child shell, if there is any
need for preparation before running a child process.  If your need
is that simple, you can just use the list form of the `system()`
function, and it will `fork()` and `exec()` the command and argument
vector.  But if you need to redirect I/O, or change directory
just prior to running the program, then it is easier to use
the form of `system()` that takes a single string, which gets passed
to a child shell for interpretation.  This leads to all sorts of
potential security problems, possibly even an extra layer of
compatibility issues.  And, by the way, it is inefficient.

You could call `fork()`, then do some pre-exec() preparation of
your own, then call `exec()` and `wait()` explicitly.  But, few people
do that.  This is not just some hypothetical.  I have seen much code
in the wild that calls `system()` with shell scripts and even $variable
substitution.

Modern Python is better.  It has subprocess.call(), which allows
you to redirect stdin, stdout and stderr.

Even those who try to be a bit more careful would have a hard
time duplicating the error checking and reporting that would be
required to do the job correctly.

So, you have two choices:

1. Use `fork()` and `exec()` and you are on your own for any pre-exec code, or

2. Punt, and run a child shell.

The micro-shell is meant to be a third option, with functionality
somewhere in the middle.  The idea is to make it relatively easy
to do the right thing.

## Implementation

The bulk of `ush` is the library, `libush`.
The `ush` command is a trivial program.
It takes about half a dozen lines of code to implement the
`ush` program, which just calls `ush_argv(...)`.

A C program can call `ush_argv()` directly, so there is no need
to run the `ush` program.  The reason `ush`, as a standalone
command, exists at all is that it makes for a handy interpreter
for small jobs that might otherwise be handled by shell.

I myself use /usr/local/bin/ush as an interpreter for light-weight
tasks that might otherwise have been shell functions, or aliases
or trivial size shell scripts.  But, sometimes, `ush` is safer,
or is more robust with respect to non-printing characters or
shell magic characters in the input or arguments.


Like most shells, `ush` can be told to run commands directly from
the command line, using `--command`, or without the `--command`
option, its first non-option argument is the path to a script.

All `ush` commands are in the form of options.  There is no `ush`
command language or special syntax, only command-line options.

For example, to change directory before running the child program,
you specify `--chdir <directory>` on the command line.

Order of options matters.  Options are not reordered, as is the case with
many GNU-style programs.  Commands are executed in order of appearance on the
command line.  Although `ush` commands, like `--chdir` look like options,
they are best thought of as commands that just happen to have option syntax.
If, for example, `ush` is given

```bash
  --chdir=/tmp -- stdout=tmp.out
```

the result is different from `--stdout` followed by `--chdir`.


## Scripts

A `ush` consists of lines which are the exact same structure as
a command-line argument vector, except that options, the command
path and the arguments are in a file, one per line.
First comes all the option commands, executed in order;
then comes a line consisting of nothing but '--' to end the options;
then comes the command path followed by all the arguments.

## Encoding of scripts

One thing different about parsing of options from a script
is that there is a choice of encodings.  The "lines" read in
can be plain text (default), or null-terminated (--encoding=null)
or quoted-printable (--encoding=qp) or xnn.

Null and qouted-printable are widely known, even standards.
Xnn encoding I just made up.

## Why another encoding?

The reason for going with yet another encoding is that I like
to easily edit scripts with a text editor, and use all the Unix
tools to examine and manipulate a text file.  Null-terminated
strings are partly there, but have not really "arrived".
So, I tried quote-printable, and found that many many lines
that looked perfectly reasonable had to be encoded, simply
because the equal sign is so common.  Encoding all lines
of the form  `--option=value`  to  become  `--option=3Dvalue`
got to be tiresome.

## Error reporting

`libush` uses `libexplain`.  A significant fraction of the
code in `libush` is dedicated to properly reporting errors.
When something goes wrong, the error report included not only
the errno and the arguments to some function, but also to
the real cause of the problem.  Any failure of a system call
or library function results in reporting the numeric and the
symbolic value of errno, and the perror()-style string.

For example, if opening a file fails due to permission problems,
the true underlying cause could be not the permissions of the
file itself, but further up the chain of directories in the
path leading up to the file.  `libexplain` will report the
directory that is the true cause of the problem.

## Options and commands

--debug

Pretty-print values of interest only for debugging.

--verbose

Show some feedback while running.

--command

Do not run a script file; execute all the non-option arguments
as a command name and its arguments

--fork

`fork()` and then run the child process and `wait()` for it.
Without this option, there is no `fork()`,
so the current process is overwritten by `exec()`.

--chdir=_dir_

Change directory to _dir_ before running the program.

--umask=_mask_

Call `umask()` before running the program.
_mask_ can be an octal number or it can be symbolic.
Symbolic values for mask are like those reported by
the command, `umask -S`.  That is, some subset of

    u=rwx,g=rwx,o=rwx

#### I/O redirection

--stdin=_path_

Redirect stdin (fd 0) to the given <path>, for reading.

--stdout=_path_

Redirect stdout (fd 1) to the given <path>, for writing.

--stderr=_path_

Redirect stderr (fd 2) to the given <path>, for writing.

The variations for stdout and stderr are:

  --stdout-append=_path_
  --stderr-append=_path_

  --stdout-new=_path_
  --stdout-new=_path_

The -new variants assert that the file does not already exist.
Often, it is important to guard against accidentally writing
over an existing file.


#### Script files

--append-argv

Copy any remaining non-option arguments
to the end of the argument list of the command to be run.

This applies only to interpreting a script file.

--replace=_string_

If any argument in a script exactly matches the given _string_,
then splice in all the remaining non-option arguments
in pace of that one argument.  Sort of like what `xargs` does.

This applies only to interpreting a script file.

--encoding=_encoding_
  where _encoding_ is one of { test | xnn | null | qp }.

When reading a script file, read each line
or null-terminated record, and decode it,
according the given encoding.

This applies only to interpreting a script file.

## Environment

If either of the environment variables, `USH_DEBUG` and `USH_VERBOSE`
is present, then it is as if the `--debug` or `--verbose` options
were given on the command-line or in the command/option section
of a script.
This is because it can be easier to turn on and off in the
environment, rather than edit a script file.

## Dependencies

1. libush
2. libcscript
3. libexplain

#### libush
`ush` is both a library and a standalone command.
`libush` is the collection of functions specific to `ush`.

#### libcscript
There is such a library as `libcscript`.
It exists to support a project called "CScript".
It has some generally useful functions.
For the micro-shell, rather than refer to that library,
it does re-use by copying just the subset that it needs
into its own subdirectory, `libcscript`.  This is done by
shaking out parts that are referred to in the object files
just before link time.  What is contained in `libcscript`
in this Github repository is a "frozen" version of that subset.

The idea is that, although re-use by copy-and-paste is
considered to be a bad thing, in general, it is a good trade-off
for software that is to be distributed, because there are few
developers and possibly many "customers".  It would be bad manners
to stick them with some new "dependency hell".

#### libexplain
For now, `ush` is hard-coded to use `libexplain`.
Later, I may convert to a shim layer that can either use
`libexplain` or fall back to a simpler built-in explainer.

## Portability
There is nothing inherently Linux-specific about `ush`,
but I have not tested it on other Unix-like platforms.

## Cleanliness
All of `ush`, `libush` and `lincscript` have been written in
a subset of C and C++.  It compiles clean using a "super-compiler"
that compiles using:

    1. gcc -Wall -Wextra
    2. clang
    3. g++
    4. clang++

## Coding style
The coding style used for `ush` can most briefly be described as Sun cstyle, but with certain changes.
They are:
    + tabs instead of spaces
    + 4 character indent, instead of 8
    + no cuddling of braces

## No limits

`libush` has no arbitrary limits.  There is no limit imposed
by `libush` due to fixed size buffers or arrays, etc.
Line buffers grow as needed.  Arrays for things like
arguments grow as needed.

## Examples


### Shell one-liner

```Bash

ush --chdir=/tmp --stdout=tmp-date --command -- date

```

### Calling a `libush` function from C code

```C

    char *cmd_argv[] = {
        "--fork",
        "--chdir=/tmp",
        "--stdout=tmp-date",
        "--command",
        "--",
        "date",
        NULL
    };

    ush_argv(6, cmd_argv);
```

### As a script ...

```Bash

/path/to/script

```

where script is:


```Bash

#! /usr/local/bin/ush --encoding=xnn

--append-argv
--
ls
--color=tty
--time-style=+%Y-%m-%d\x20%H:%M

```

Notice the `--encoding=xnn`.
In this case it is not really necessary,
because the `\x20` in the time-style argument is not really
necessary, because `ush` treats each whole line as an argument,
and spaces are nothing special.  But, an explicit`\x20`
makes it more clear the reader that this is a single argument
with a space in it.

Note also that quote marks do not do any good here.
They do not serve to ensure that the time-style argument
is not broken into words.  That does not happen in the first place.
Quote marks of any kind are not special.

-- Guy Shaw

   gshaw@acm.org

