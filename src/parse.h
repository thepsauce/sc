#ifndef PARSE_H
#define PARSE_H

#include "core.h"

#include <stdio.h>
#include <gmp.h>

extern struct parser {
    /* start (null terminated), pointer (current) */
    char *s, *p;

    /* read_word (uses n for length) */
    char *w;
    /* read_number (overwrites n as well) */
    mpf_t f;
    int n;

    /* stack */
    struct group *st[1024];
    /* stack pointer */
    int sp;
} Parser;

#define PARSER_ERROR (-1)
#define PARSER_OK (0)
#define PARSER_CONTINUE (1)

int init_parser(void);
void reset_parser(void);
int parse(const char *s);

#endif
