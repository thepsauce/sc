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

static void read_expression(int p /* precedence */, struct group *g)
{
    /* opr expr */
    static const struct prefix_operator {
        const char *s;
        enum group_type t;
        int p;
    } prefixes[] = {
        { "+", GROUP_POSITIVE, 5 },
        { "-", GROUP_NEGATE, 5 },
        { "negate", GROUP_NEGATE, 5 },
        { "not", GROUP_NOT, 4 },
    };

    /* expr opr expr */
    static const struct infix_operator {
        const char *s;
        enum group_type t;
        int p;
    } infixes[] = {
        { "*", GROUP_MULTIPLY, 6 },
        { "+", GROUP_PLUS, 5 },
        { ",", GROUP_COMMA, 1 },
        { "-", GROUP_MINUS, 5 },
        { "/", GROUP_DIVIDE, 6 },
        { ";", GROUP_SEMICOLON, 2 },
        { "<=", GROUP_LESS_THAN, 4 },
        { "<", GROUP_LESS, 4 },
        { "=", GROUP_EQUALS, 4 },
        { ">=", GROUP_GREATER_THAN, 4 },
        { ">", GROUP_GREATER, 4 },
        { "^", GROUP_RAISE, 7 },
        { "_", GROUP_LOWER, 10 },
        { "≠", GROUP_NOT_EQUALS, 4 },
        { "≤", GROUP_LESS_THAN, 4 },
        { "≥", GROUP_GREATER_THAN, 4 },
        { "and", GROUP_AND, 3 },
        { "do", GROUP_DO, 7 },
        { "equals", GROUP_EQUALS, 3 },
        { "greater than", GROUP_GREATER_THAN, 3 },
        { "greater", GROUP_GREATER, 3 },
        { "if", GROUP_IF, 2 },
        { "less than", GROUP_LESS_THAN, 3 },
        { "less", GROUP_LESS, 3 },
        { "mod", GROUP_MOD, 6 },
        { "or", GROUP_OR, 3 },
        { "where", GROUP_WHERE, 1 },
        { "xor", GROUP_XOR, 3 },

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
        { "!", GROUP_EXCLAM, 8 },
        { "%", GROUP_PERCENT, 8 },
        { "else", GROUP_ELSE, 2 },
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

    bool hasValue = false;

    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(prefixes); i++) {
        const size_t n = begins_with(prefixes[i].s);
        if (n > 0) {
            Parser.p += n;
            g->t = prefixes[i].t;
            g->g = new_group(1);
            g->n = 1;
            read_expression(prefixes[i].p, g->g);
            skip_space();
            goto got_value;
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
            read_expression(uniques[i].p, &g->g[0]);
            skip_space();
            const size_t n = begins_with(uniques[i].s2);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        uniques[i].s2, uniques[i].s1);
            }
            Parser.p += n;
            read_expression(uniques[i].p, &g->g[1]);
            g->n = 2;
            goto got_value;
        }
    }

try_another_value:
    for (size_t i = 0; i < ARRAY_SIZE(matches); i++) {
        const size_t n = begins_with(matches[i].l);
        if (n > 0) {
            if (matches[i].n > 0 && strcmp(matches[i].l, matches[i].r) == 0) {
                return;
            }

            Parser.p += n;

            struct group *c = new_group(1);
            matches[i].n++;
            read_expression(0, c);
            matches[i].n--;
            if (g->t != GROUP_NULL) {
                surround_group(c, matches[i].t);
                if (g->t != GROUP_IMPLICIT) {
                    surround_group(g, GROUP_IMPLICIT);
                }
                join_group(g, c);
            } else {
                g->t = matches[i].t;
                g->g = c;
                g->n = 1;
            }

            skip_space();
            const size_t n = begins_with(matches[i].r);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        matches[i].r, matches[i].l);
            }
            Parser.p += n;
            goto got_value;
        }
    }

    struct group t;

    if (isalpha(Parser.p[0])) {
        read_word();
        t.t = GROUP_VARIABLE;
        t.v.w = dict_putl(Parser.w, Parser.n);
        if (t.v.w == NULL) {
            throw_error("%s", strerror(errno));
        }
    } else if (isdigit(Parser.p[0]) || Parser.p[0] == '.') {
        read_number();
        t.t = GROUP_NUMBER;
        mpf_init_set(t.v.f, Parser.f);
    } else {
        if (hasValue) {
            return;
        }
        throw_error("need value");
    }

    if (g->t != GROUP_NULL) {
        if (g->t != GROUP_IMPLICIT) {
            surround_group(g, GROUP_IMPLICIT);
        }
        join_group_no_free(g, &t);
    } else {
        *g = t;
    }

got_value:
    hasValue = true;

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
            read_expression(infixes[i].p, r);
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

    /* 6 is the precedence for multiplication */
    if (p <= 6) {
        goto try_another_value;
    }
}

int parse(struct group *g)
{
    mpf_init(Parser.f);
    if (setjmp(Parser.jb) > 0) {
        return -1;
    }
    Parser.p = Parser.s;
    read_expression(0, g);
    return 0;
}
