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

int init_parser(void)
{
    if ((Parser.st[0] = new_group(1)) == NULL) {
        return PARSER_ERROR;
    }
    Parser.sp = 1;
    mpf_init(Parser.f);
    return PARSER_OK;
}

void reset_parser(void)
{
    clear_group(Parser.st[0]);
    Parser.st[0]->t = GROUP_NULL;
    Parser.st[0]->g = NULL;
    Parser.st[0]->n = 0;
    Parser.sp = 1;
}

static void indicate_error(const char *msg, ...)
{
    while (Parser.p != Parser.s) {
        Parser.p--;
        fprintf(stderr, " ");
    }
    fprintf(stderr, "    ~\n");

    va_list l;
    va_start(l, msg);
    vfprintf(stderr, msg, l);
    va_end(l);
    fputc('\n', stderr);
}

static size_t skip_space(void)
{
    size_t n = 0;
    while (isspace(Parser.p[0])) {
        Parser.p++;
        n++;
    }
    return n;
}

static int read_word(void)
{
    if (!isalpha(Parser.p[0])) {
        indicate_error("expected word");
        return PARSER_ERROR;
    }
    Parser.w = Parser.p;
    while (isalpha(Parser.p[0])) {
        Parser.p++;
    }
    Parser.n = Parser.p - Parser.w;
    return PARSER_OK;
}

static int read_number(void)
{
    if (gmp_sscanf(Parser.p, "%Ff%n", Parser.f, &Parser.n) != 1) {
        indicate_error("expected number");
        return PARSER_ERROR;
    }
    Parser.p += Parser.n;
    return PARSER_OK;
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

static const int Precedences[] = {
    [GROUP_NULL] = 0,

    [GROUP_ROUND] = 0,
    [GROUP_DOUBLE_CORNER] = 0,
    [GROUP_CORNER] = 0,
    [GROUP_SQUARE] = 0,
    [GROUP_CURLY] = 0,
    [GROUP_DOUBLE_BAR] = 0,
    [GROUP_BAR] = 0,

    [GROUP_SEMICOLON] = 1,
    [GROUP_COMMA] = 2,

    [GROUP_IF] = 3,
    [GROUP_DO] = 3,
    [GROUP_WHERE] = 3,
    [GROUP_ELSE] = 3,

    [GROUP_NOT] = 4,

    [GROUP_AND] = 4,
    [GROUP_OR] = 4,
    [GROUP_XOR] = 4,

    [GROUP_LESS] = 5,
    [GROUP_LESS_EQUAL] = 5,
    [GROUP_GREATER] = 5,
    [GROUP_GREATER_EQUAL] = 5,
    [GROUP_EQUAL] = 5,
    [GROUP_NOT_EQUAL] = 5,

    [GROUP_POSITIVE] = 6,
    [GROUP_NEGATE] = 6,

    [GROUP_PLUS] = 6,
    [GROUP_MINUS] = 6,

    [GROUP_MULTIPLY] = 7,
    [GROUP_DIVIDE] = 7,
    [GROUP_MOD] = 7,

    [GROUP_IMPLICIT] = 7,

    [GROUP_ELEMENT_OF] = 8,

    [GROUP_SQRT] = 9,
    [GROUP_CBRT] = 9,

    [GROUP_RAISE] = 9,
    [GROUP_RAISE2] = 9,
    [GROUP_RAISE3] = 9,

    [GROUP_LOWER] = 10,

    [GROUP_EXCLAM] = 11,
    [GROUP_PERCENT] = 11,

    [GROUP_VARIABLE] = INT_MAX,
    [GROUP_NUMBER] = INT_MAX,
};

struct group *walk_up_precedences(int p)
{
    while (Parser.sp > 1) {
        if (Precedences[Parser.st[Parser.sp - 2]->t] <= p) {
            break;
        }
        Parser.sp--;
    }
    return Parser.st[Parser.sp - 1];
}

int parse(const char *s)
{
    /* opr expr */
    static const struct prefix_operator {
        const char *s;
        enum group_type t;
    } prefixes[] = {
        { "+", GROUP_POSITIVE },
        { "-", GROUP_NEGATE },
        { "negate", GROUP_NEGATE },
        { "not", GROUP_NOT },
        { "√", GROUP_SQRT },
        { "∛", GROUP_CBRT },
    };

    /* expr opr expr */
    static const struct infix_operator {
        const char *s;
        enum group_type t;
    } infixes[] = {
        { ";", GROUP_SEMICOLON },

        { ",", GROUP_COMMA },

        { "where", GROUP_WHERE },
        { "if", GROUP_IF },
        { "do", GROUP_DO },

        { "and", GROUP_AND },
        { "or", GROUP_OR },
        { "xor", GROUP_XOR },

        { "equals", GROUP_EQUAL },
        { "greater equals", GROUP_GREATER_EQUAL },
        { "greater than", GROUP_GREATER },
        { "less equals", GROUP_LESS_EQUAL },
        { "less than", GROUP_LESS },

        { "<=", GROUP_LESS_EQUAL },
        { "<", GROUP_LESS },
        { "=", GROUP_EQUAL },
        { ">=", GROUP_GREATER_EQUAL },
        { ">", GROUP_GREATER },
        { "≠", GROUP_NOT_EQUAL },
        { "≤", GROUP_LESS_EQUAL },
        { "≥", GROUP_GREATER_EQUAL },

        { "+", GROUP_PLUS },
        { "-", GROUP_MINUS },
        { "*", GROUP_MULTIPLY },
        { "·", GROUP_MULTIPLY },
        { "/", GROUP_DIVIDE },
        { "mod", GROUP_MOD },

        { "∈", GROUP_ELEMENT_OF },

        { "^", GROUP_RAISE },
        { "_", GROUP_LOWER },
    };

    /* expr opr */
    static const struct suffix_operator {
        const char *s;
        enum group_type t;
    } suffixes[] = {
        { "!", GROUP_EXCLAM },
        { "%", GROUP_PERCENT },
        { "²", GROUP_RAISE2 },
        { "³", GROUP_RAISE3 },
        { "else", GROUP_ELSE },
    };

    /* make sure these have the same addresses */
    static const char *const bar = "|";
    static const char *const doubleBar = "||";
    static const char *const doubleBar2 = "‖";

    /* opr expr opr */
    static const struct match_operator {
        const char *l, *r;
        enum group_type t;
    } matches[] = {
        { "(", ")", GROUP_ROUND },
        { "«", "»", GROUP_DOUBLE_CORNER },
        { "<<", ">>", GROUP_DOUBLE_CORNER },
        { "<", ">", GROUP_CORNER },
        { "[", "]", GROUP_SQUARE },
        { "{", "}", GROUP_CURLY },
        { doubleBar2, doubleBar2, GROUP_DOUBLE_BAR },
        { doubleBar, doubleBar, GROUP_DOUBLE_BAR },
        { bar, bar, GROUP_BAR },
    };

    struct group *g;

    Parser.s = (char*) s;
    Parser.p = Parser.s;
    g = Parser.st[Parser.sp - 1];

beg:
    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(prefixes); i++) {
        const size_t n = begins_with(prefixes[i].s);
        if (n > 0) {
            Parser.p += n;
            g = surround_group(g, prefixes[i].t, 1);
            if (g == NULL) {
                return PARSER_ERROR;
            }
            Parser.st[Parser.sp++] = g;
            goto beg;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(matches); i++) {
        const size_t n = begins_with(matches[i].l);
        if (n > 0) {
            Parser.p += n;
            g = surround_group(g, matches[i].t, 1);
            if (g == NULL) {
                return PARSER_ERROR;
            }
            Parser.st[Parser.sp++] = g;
            goto beg;
        }
    }

    if (isalpha(Parser.p[0])) {
        read_word();
        g->t = GROUP_VARIABLE;
        g->v.w = dict_putl(Parser.w, Parser.n);
        if (g->v.w == NULL) {
            indicate_error("%s", strerror(errno));
            return PARSER_ERROR;
        }
    } else if (isdigit(Parser.p[0]) || Parser.p[0] == '.') {
        read_number();
        g->t = GROUP_NUMBER;
        mpf_init_set(g->v.f, Parser.f);
    } else if (Parser.p[0] != '\0') {
        g->t = GROUP_VARIABLE;
        Parser.w = Parser.p;
        if (!(Parser.p[0] & 0x80)) {
            Parser.p++;
        } else {
            while (Parser.p++, (Parser.p[0] & 0xc0) == 0x80) {
                (void) 0;
            }
        }
        Parser.n = Parser.p - Parser.w;
        g->v.w = dict_putl(Parser.w, Parser.n);
        if (g->v.w == NULL) {
            indicate_error("%s", strerror(errno));
            return PARSER_ERROR;
        }
    } else {
        return PARSER_CONTINUE;
    }

infix:
    skip_space();
    if (Parser.p[0] == '\0') {
        return PARSER_OK;
    }

    for (size_t i = 0; i < ARRAY_SIZE(infixes); i++) {
        const size_t n = begins_with(infixes[i].s);
        if (n > 0) {
            g = walk_up_precedences(Precedences[infixes[i].t]);
            Parser.p += n;
            g = surround_group(g, infixes[i].t, 2);
            if (g == NULL) {
                return PARSER_ERROR;
            }
            Parser.st[Parser.sp++] = g;
            goto beg;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(suffixes); i++) {
        const size_t n = begins_with(suffixes[i].s);
        if (n > 0) {
            g = walk_up_precedences(Precedences[suffixes[i].t]);
            Parser.p += n;
            if (surround_group(g, suffixes[i].t, 1) == NULL) {
                return PARSER_ERROR;
            }
            goto infix;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(matches); i++) {
        const size_t n = begins_with(matches[i].r);
        if (n > 0) {
            /* find matching left bracket */
            int sp = Parser.sp;
            if (Precedences[g->t] == 0) {
                sp--;
            }
            while (sp > 0) {
                if (Precedences[Parser.st[sp - 1]->t] == 0) {
                    break;
                }
                sp--;
            }
            if (sp == 0) {
                indicate_error("not open: '%s' needs matching '%s'",
                        matches[i].r, matches[i].l);
                return PARSER_ERROR;
            }
            struct group *const left = Parser.st[sp - 1];
            if (left->t != matches[i].t) {
                if (matches[i].l == matches[i].r) {
                    goto beg;
                }
                indicate_error("collision: a previous pair needs to be closed first");
                return PARSER_ERROR;
            }

            Parser.p += n;
            Parser.sp = sp;
            g = left;
            goto infix;
        }
    }

    /* implicit operator */
    /* for example: 2a, 2 2, a bc */
    g = walk_up_precedences(Precedences[GROUP_IMPLICIT]);
    g = surround_group(g, GROUP_IMPLICIT, 2);
    if (g == NULL) {
        return PARSER_ERROR;
    }
    Parser.st[Parser.sp++] = g;

    goto beg;
}
