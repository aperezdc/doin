/*
 * doin.c
 * Copyright (C) 2015 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#define _GNU_SOURCE 1
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sched.h>

#define LENGTH_OF(_v)  (sizeof (_v) / sizeof ((_v)[0]))


static const struct {
    const char *name;
    int         flag;
} s_namespaces[] = {
    { "pid",  CLONE_NEWPID  },
    { "ipc",  CLONE_NEWIPC  },
    { "net",  CLONE_NEWNET  },
    { "uts",  CLONE_NEWUTS  },
    { "mnt",  CLONE_NEWNS   },
    { "user", CLONE_NEWUSER },
};


static int
doin_error_main (int argc, char **argv, char **envp)
{
    (void) argc; /* unused */;
    (void) argv; /* unused */;
    (void) envp; /* unused */;

    fprintf (stderr, "%s: %s '%s': %s\n", argv[0], argv[1], argv[2], argv[3]);
    return EXIT_FAILURE;
}


typedef int (*start_main_func) (int (*main) (int, char**, char**),
                                int argc, char **ubp_av,
                                void (*init) (void),
                                void (*fini) (void),
                                void (*rtld_fini) (void),
                                void (*stack_end));

int __libc_start_main(int (*main) (int, char**, char**),
                      int argc, char ** ubp_av,
                      void (*init) (void),
                      void (*fini) (void),
                      void (*rtld_fini) (void),
                      void (*stack_end))
{
    start_main_func orig_libc_start_main = dlsym (RTLD_NEXT, "__libc_start_main");
    char *error_argv[] = { "doin", "error-kind", "error-file", "error-message", NULL };

    const char *pid_string = getenv ("__DOIN_ATTACH_PID");
    if (!pid_string) {
        error_argv[1] = "getenv";
        error_argv[2] = "__DOIN_ATTACH_PID";
        error_argv[3] = "no environment variable";
        ubp_av = error_argv;
        main = doin_error_main;
        goto call_libc_start_main;
    }

    char path[PATH_MAX + 1];
    for (unsigned i = 0; i < LENGTH_OF (s_namespaces); i++) {
        snprintf (path, PATH_MAX, "/proc/%s/ns/%s", pid_string, s_namespaces[i].name);

        /*
         * For error handling, we have to make sure that our function can
         * exit() properly, which means it has to be done inside main().
         * We have to overwrite main with our own, which does the error
         * reporting.
         */
        int fd = open (path, O_RDONLY);
        if (fd < 0) {
            error_argv[1] = "cannot open";
            error_argv[2] = path;
            error_argv[3] = strerror (errno);
            ubp_av = error_argv;
            main = doin_error_main;
            break;
        }
        if (setns (fd, s_namespaces[i].flag) < 0) {
            error_argv[1] = "setns()";
            error_argv[2] = (char*) s_namespaces[i].name;
            error_argv[3] = strerror (errno);
            close (fd);
            ubp_av = error_argv;
            main = doin_error_main;
            break;
        }
    }

call_libc_start_main:
    return (*orig_libc_start_main) (main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
