#include "dict.h"
#include "parse.h"
#include "macros.h"

#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <wchar.h>

struct parser Parser;

void throw_error(const char *msg, ...)
{
    while (Parser.p != Parser.s) {
        Parser.p--;
        fprintf(stderr, " ");
    }
    fprintf(stderr, "~\n");

    va_list l;
    va_start(l, msg);
    vfprintf(stderr, msg, l);
    va_end(l);
    fprintf(stderr, "\n");

    longjmp(Parser.jb, 0);
}

static void skip_space(void)
{
    while (isspace(Parser.p[0])) {
        Parser.p++;
    }
}

static void read_word(void)
{
    if (!isalpha(Parser.p[0])) {
        throw_error("expected word");
    }
    Parser.w = Parser.p;
    while (isalpha(Parser.p[0])) {
        Parser.p++;
    }
    Parser.n = Parser.p - Parser.w;
}

static void read_number(void)
{
    if (gmp_sscanf(Parser.p, "%Ff%n", Parser.f, &Parser.n) != 1) {
        throw_error("expected number");
    }
    Parser.p += Parser.n;
}

static size_t begins_with(const char *s)
{
    size_t n = 0;
    for (; *s != '\0'; s++) {
        if (*s != Parser.p[n]) {
            return 0;
        }
        n++;
    }
    return n;
}

#define PREC_MULTIPLY 7

static void read_expression(bool e, int p /* precedence */, struct group *g)
{
    /* opr expr */
    static const struct prefix_operator {
        const char *s;
        enum group_type t;
        int p;
    } prefixes[] = {
        { "+", GROUP_POSITIVE, 6 },
        { "-", GROUP_NEGATE, 6 },
        { "negate", GROUP_NEGATE, 6 },
        { "not", GROUP_NOT, 4 },
    };

    /* expr opr expr */
    static const struct infix_operator {
        const char *s;
        enum group_type t;
        int p;
    } infixes[] = {
        { ";", GROUP_SEMICOLON, 1 },

        { ",", GROUP_COMMA, 2 },

        { "where", GROUP_WHERE, 3 },
        { "if", GROUP_IF, 3 },

        { "and", GROUP_AND, 4 },
        { "or", GROUP_OR, 4 },
        { "xor", GROUP_XOR, 4 },

        { "equals", GROUP_EQUALS, 5 },
        { "greater than", GROUP_GREATER_THAN, 5 },
        { "greater", GROUP_GREATER, 5 },
        { "less than", GROUP_LESS_THAN, 5 },
        { "less", GROUP_LESS, 5 },
        { "<=", GROUP_LESS_THAN, 5 },
        { "<", GROUP_LESS, 5 },
        { "=", GROUP_EQUALS, 5 },
        { ">=", GROUP_GREATER_THAN, 5 },
        { ">", GROUP_GREATER, 5 },
        { "≠", GROUP_NOT_EQUALS, 5 },
        { "≤", GROUP_LESS_THAN, 5 },
        { "≥", GROUP_GREATER_THAN, 5 },

        { "-", GROUP_MINUS, 6 },
        { "+", GROUP_PLUS, 6 },
        { "*", GROUP_MULTIPLY, PREC_MULTIPLY },
        { "/", GROUP_DIVIDE, PREC_MULTIPLY },
        { "mod", GROUP_MOD, PREC_MULTIPLY },

        { "^", GROUP_RAISE, 8 },
        { "do", GROUP_DO, 8 },

        { "_", GROUP_LOWER, 10 },

        /* from is here because it is part of choose */
        /* (-1 means this will always be passed to the parent) */
        { "from", 0, -1 },
    };

    /* expr opr */
    static const struct suffix_operator {
        const char *s;
        enum group_type t;
        int p;
    } suffixes[] = {
        { "!", GROUP_EXCLAM, 9 },
        { "%", GROUP_PERCENT, 9 },
        { "else", GROUP_ELSE, 3 },
    };

    /* opr expr opr */
    static /* no const */ struct match_operator {
        const char *l, *r;
        enum group_type t;
        size_t n; /* number of open brackets */
        /* (important when it ends and starts with
         * the same symbol)
         */
    } matches[] = {
        { "(", ")", GROUP_ROUND, 0 },
        { "<<", ">>", GROUP_DOUBLE_CORNER, 0 },
        { "<", ">", GROUP_CORNER, 0 },
        { "[", "]", GROUP_SQUARE, 0 },
        { "{", "}", GROUP_CURLY, 0 },
        { "||", "||", GROUP_DOUBLE_BAR, 0 },
        { "|", "|", GROUP_BAR, 0 },
    };

    /* opr expr opr expr */
    static const struct unique_operator {
        const char *s1, *s2;
        enum group_type t;
        int p;
    } uniques[] = {
        { "choose", "from", GROUP_CHOOSE_FROM, 4 },
    };

    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(prefixes); i++) {
        const size_t n = begins_with(prefixes[i].s);
        if (n > 0) {
            Parser.p += n;
            g->t = prefixes[i].t;
            g->g = new_group(1);
            g->n = 1;
            read_expression(false, prefixes[i].p, g->g);
            skip_space();
            goto next_infix;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(uniques); i++) {
        const size_t n = begins_with(uniques[i].s1);
        if (n > 0) {
            if (uniques[i].p <= p) {
                return;
            }
            Parser.p += n;
            g->t = uniques[i].t;
            g->g = new_group(2);
            read_expression(false, uniques[i].p, &g->g[0]);
            skip_space();
            const size_t n = begins_with(uniques[i].s2);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        uniques[i].s2, uniques[i].s1);
            }
            Parser.p += n;
            read_expression(false, uniques[i].p, &g->g[1]);
            g->n = 2;
            goto next_infix;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(matches); i++) {
        const size_t n = begins_with(matches[i].l);
        if (n > 0) {
            if (matches[i].n > 0 && strcmp(matches[i].l, matches[i].r) == 0) {
                return;
            }

            Parser.p += n;

            g->t = matches[i].t;
            g->g = new_group(1);
            g->n = 1;

            matches[i].n++;
            read_expression(false, 0, g->g);
            matches[i].n--;

            skip_space();
            const size_t n = begins_with(matches[i].r);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        matches[i].r, matches[i].l);
            }
            Parser.p += n;
            goto next_infix;
        }
    }

    if (isalpha(Parser.p[0])) {
        read_word();
        g->t = GROUP_VARIABLE;
        g->v.w = dict_putl(Parser.w, Parser.n);
        if (g->v.w == NULL) {
            throw_error("%s", strerror(errno));
        }
    } else if (isdigit(Parser.p[0]) || Parser.p[0] == '.') {
        read_number();
        g->t = GROUP_NUMBER;
        mpf_init_set(g->v.f, Parser.f);
    } else {
        if (e) {
            return;
        }
        throw_error("need value");
    }

next_infix:
    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(infixes); i++) {
        const size_t n = begins_with(infixes[i].s);
        if (n > 0) {
            if (infixes[i].p <= p) {
                return;
            }
            Parser.p += n;
            struct group *r = new_group(1);
            read_expression(false, infixes[i].p, r);
            if (g->t != infixes[i].t) {
                surround_group(g, infixes[i].t);
            }
            join_group(g, r);
            goto next_infix;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(suffixes); i++) {
        const size_t n = begins_with(suffixes[i].s);
        if (n > 0) {
            if (suffixes[i].p <= p) {
                return;
            }
            surround_group(g, suffixes[i].t);
            Parser.p += n;
            goto next_infix;
        }
    }

    if (p < PREC_MULTIPLY) {
        struct group *r = new_group(1);
        read_expression(true, PREC_MULTIPLY, r);
        if (r->t != GROUP_NULL) {
            if (g->t != GROUP_IMPLICIT) {
                surround_group(g, GROUP_IMPLICIT);
            }
            join_group(g, r);
        } else {
            free(r);
            return;
        }
        goto next_infix;
    }
}

int parse(struct group *g)
{
    mpf_init(Parser.f);
    if (setjmp(Parser.jb) > 0) {
        return -1;
    }
    Parser.p = Parser.s;
    read_expression(false, 0, g);
    return 0;
}
