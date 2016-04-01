# ush -- micro-shell -- a very light-weight "shell" library and command

The micro-shell, 'ush', is more or less like the exec() and
system() functions that exist in C and in higher-level languages
like Perl, Python, Ruby, etc.  The main difference is that
|ush| has commands to do things like redirect I/O and change directory,
just before running the command.

The reason this is important is that system(), for example in Perl,
pretty much encourages people to run a child shell, if there is any
need for preparation before running a child process.  If your need
is that simple, you can just use the list form of the system()
function, and it will fork() and exec() the command and argument
vector.  But if you need to redirect I/O, then it is easier to use
the form of system() that takes a single string, which gets passed
to a child shell for interpretation.  This leads to all sorts of
potential security problems, possibly even an extra layer of
compatibility issues.  And, by the way, it is inefficient.

You could call fork(), then do some pre-exec() preparation of
your own, then call exec() and wait() explicitly.  But, few people
do that.  This is not just some hypothetical.  I have seen much code
in the wild that calls system() with shell scripts and even $variable
substitution.

Even those who try to be a bit more careful would have a hard
time duplicating the error checking and reporting that would be
required to do the job correctly.

So, you have two choices:

  1) use fork() and exec() and you are on your own
     for any pre-exec() code, or

  2) Punt, and run a child shell.

The micro-shell is meant to be a third option, with functionality
somewhere in the middle.  The idea is to make it relatively easy
to do the right thing.


# Implementation

The bulk of |ush| is the library, |libush|.
The |ush| command is a trivial program.
It takes about half a dozen lines of code to implement the
|ush| program, which just calls ush_argv(...).

A C program can call ush_argv() directly, so there is no need
to run the |ush| program.  The reason |usha,| as a standalone
command, exists at all is that it makes for a handy interpreter
for small jobs that might otherwise be handled by shell.

I myself use /usr/local/bin/ush as an interpreter for light-weight
tasks that might otherwise have been shell functions, or aliases
or trivial size shell scripts.  But, sometimes, |ush| is safer,
or is more robust with respect to non-printing characters or
shell magic characters in the input or arguments.


Like most shells, |ush| can be told to run commands directly from
the command line, using |--command|, or without the --command
option, its first non-option argument is the path to a script.

All |ush| commands are in the form of options.  There is no |ush|
command language or special syntax, only command-line options.

For example, to change directory before running the child program,
you specify --chdir <directory> on the command line.

Order of options matters.  Options are not reordered, as is the case with
many GNU-style programs.  Commands are executed in order of appearance on the
command line.  Although |ush| commands, like '--chdir' look like options,
they are best thought of as commands that just happen to have option syntax.
If, for example, |ush| is given

  --chdir=/tmp -- stdout=tmp.out

the result is different from |--stdout| followed by |--chdir|.


# Scripts

A |ush| consists of lines which are the exact same structure as
a command-line argument vector, except that options, the command
path and the arguments are in a file, one per line.
First comes all the option commands, executed in order;
then comes a line consisting of nothing but '--' to end the options;
then comes the command path followed by all the arguments.

# Encoding of scripts

One thing different about parsing of options from a script
is that there is a choice of encodings.  The "lines" read in
can be plain text (default), or null-terminated (--encoding=null)
or quoted-printable (--encoding=qp) or xnn.

Null and qouted-printable are widely known, even standards.
Xnn encoding I just made up.

# Why another encoding?

The reason for going with yet another encoding is that I like
to easily edit scripts with a text editor, and use all the Unix
tools to examine and manipulate a text file.  Null-terminated
strings are partly there, but have not really "arrived".
So, I tried quote-printable, and found that many many lines
that looked perfectly reasonable had to be encoded, simply
because the equal sign is so common.  Encoding all lines
of the form --option=value  to  become  --option=3Dvalue
got to be tiresome.

# Error reporting

|libush| uses |libexplain|.  A significant fraction of the
code in |libush| is dedicated to properly reporting errors.
When something goes wrong, the error report included not only
the errno and the arguments to some function, but also to
the real cause of the problem.  Any failure of a system call
or library function results in reporting the numeric and the
symbolic value of errno, and the perror()-style string.

For example, if opening a file fails due to permission problems,
the true underlying cause could be not the permissions of the
file itself, but further up the chain of directories in the
path leading up to the file.  |libexplain| will report the
directory that is the true cause of the problem.

# Options and commands

--debug

Pretty-print values of interest only for debugging.

--verbose

Show some feedback while running.

--command

Do not run a script file; execute all the non-option arguments
as a command name and its arguments

--fork

fork(); then run the child process and wait() for it.
Without this option, there is no fork(),
so the current process is overwritten by exec().

--chdir=<dir>

Change directory to <dir> before running the program.

--umask=<mask>

Call umask() before running the program.
<mask> can be an octal number or it can be symbolic.
Symbolic values for mask are like those reported by
the command, |umask -S|.  That is, some subset of

    u=rwx,g=rwx,o=rwx


--stdin=<path>

Redirect stdin (fd 0) to the given <path>, for reading.

--stdout=<path>

Redirect stdout (fd 1) to the given <path>, for writing.

--stderr=<path>

Redirect stderr (fd 2) to the given <path>, for writing.

--append-argv

Copy any remaining non-option arguments
to the end of the argument list of the command to be run.

This applies only to interpreting a script file.

--replace=<string>

If any argument in a script exactly matches the given <string>,
then splice in all the remaining non-option arguments
in pace of that one argument.

This applies only to interpreting a script file.

--encoding=<test|xnn|null|qp>

When reading a script file, read each line
or null-terminated record, and decode it,
according the given encoding.

This applies only to interpreting a script file.

# No limits

|libush| has no arbitrary limits.  There is no limit imposed
by |libush| due to fixed size buffers or arrays, etc.
Line buffers grow as needed.  Arrays for things like
arguments grow as needed.

# Examples

ush --chdir=/tmp --stdout=tmp-date --command -- date

-- Guy Shaw
   gshaw@acm.org

