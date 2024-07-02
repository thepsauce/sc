#ifndef PARSE_H
#define PARSE_H

#include "core.h"

/* * * Parse * * */

extern struct parser {
    /* start (null terminated), pointer (current) */
    char *s, *p;

    /* read_word (uses n for length) */
    char *w;
    /* read_number (overwrites n as well) */
    mpf_t f;
    int n;

    /* root group */
    struct group root;
    /* deepest group, needed for continuing parser (PARSER_CONTINUE) */
    struct group *cur;
} Parser;

#define PARSER_ERROR (-1)
#define PARSER_OK (0)
#define PARSER_CONTINUE (1)

/*
 * Call this once on startup to initialize the parser.
 */
int init_parser(void);

/*
 * Resets the parser to the initial state, this frees almost all resources,
 * use this after receiving PARSER_ERROR or PARSER_OK on parse().
 */
void reset_parser(void);

/*
 * Parse given string, the resulting group will be stored in g.
 *
 * This returns PARSER_ERROR when the syntax of the string is invalid or
 * PARSER_CONTINUE when the parser could continue if only it had more input
 * (just use parse(some other string) to continue) or
 * PARSER_OK when all went normal and the parser is done.
 */
int parse(const char *s);

#endif
