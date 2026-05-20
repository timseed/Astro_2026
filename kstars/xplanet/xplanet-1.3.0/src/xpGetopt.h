#ifndef _GETOPT_H
#define _GETOPT_H

/*
  The essential bits and pieces from the GNU getopt.h header file.
  This has been a headache on all kinds of different systems which
  define getopt in different ways.
 */

#ifdef __cplusplus
extern "C" {
#endif

    extern char *optarg;
    extern int optind;

    struct option
    {
# if defined __STDC__ && __STDC__
        const char *name;
# else
        char *name;
# endif
        /* has_arg can't be an enum because some compilers complain about
           type mismatches in all the code that assumes it is an int.  */
        int has_arg;
        int *flag;
        int val;
    };

# define no_argument            0
# define required_argument      1
# define optional_argument      2

extern int getopt_long_only (int __argc, char *const *__argv,
                             const char *__shortopts,
                             const struct option *__longopts, int *__longind);

#ifdef __cplusplus
}
#endif

#endif
