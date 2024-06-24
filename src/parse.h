#ifndef PARSE_H
#define PARSE_H

#include "sc.h"

#include <setjmp.h>
#include <gmp.h>

extern struct parser {
    /* start (null terminated), pointer (current) */
    char *s, *p;

    /* read_word (uses n for length) */
    char *w;

    /* read_number (overwrites n as well) */
    mpf_t f;

    /* read_operator */
    int o;

    int n;

    Expression *e;

    jmp_buf jb;
} Parser;

int parse(void);

#endif

