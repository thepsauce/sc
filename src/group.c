#include "group.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

const int Precedences[] = {
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

struct group *new_group(size_t n)
{
    struct group *g;

    g = calloc(n, sizeof(*g));
    if (g == NULL) {
        return NULL;
    }
    return g;
}

int copy_group(struct group *dest, const struct group *src)
{
    dest->p = NULL;
    dest->t = src->t;
    dest->v = src->v;
    switch (src->t) {
    case GROUP_NUMBER:
        mpf_init_set(dest->v.f, src->v.f);
        /* fall through */
    case GROUP_VARIABLE:
        dest->g = NULL;
        dest->n = 0;
        break;
    default:
        dest->g = new_group(src->n);
        if (dest->g == NULL) {
            return -1;
        }
        dest->n = src->n;
        for (size_t i = 0; i < dest->n; i++) {
            if (copy_group(&dest->g[i], &src->g[i]) == -1) {
                free(dest->g);
                return -1;
            }
            dest->g[i].p = dest;
        }
    }
    return 0;
}

struct group *surround_group(struct group *g, enum group_type t, size_t n)
{
    struct group *c;

    c = new_group(n);
    if (c == NULL) {
        return NULL;
    }
    c[0] = *g;
    /* reparent because the pointer changes */
    for (size_t i = 0; i < g->n; i++) {
        g->g[i].p = c;
    }
    g->t = t;
    g->g = c;
    g->n = n;
    /* initialize parent */
    for (size_t i = 0; i < n; i++) {
        c[i].p = g;
    }
    return &c[n - 1];
}

void clear_group(struct group *g)
{
    for (size_t i = 0; i < g->n; i++) {
        clear_group(&g->g[i]);
    }
    if (g->t == GROUP_NUMBER) {
        mpf_clear(g->v.f);
    }
    free(g->g);
}

void free_group(struct group *g)
{
    clear_group(g);
    free(g);
}

const char *GroupTypeStrings[] = {
    [GROUP_NULL] = "NULL",
    [GROUP_POSITIVE] = "POSITIVE",
    [GROUP_NEGATE] = "NEGATE",
    [GROUP_NOT] = "NOT",
    [GROUP_PLUS] = "PLUS",
    [GROUP_MINUS] = "MINUS",
    [GROUP_MULTIPLY] = "MULTIPLY",
    [GROUP_DIVIDE] = "DIVIDE",
    [GROUP_MOD] = "MOD",
    [GROUP_RAISE] = "RAISE",
    [GROUP_LOWER] = "LOWER",
    [GROUP_AND] = "AND",
    [GROUP_OR] = "OR",
    [GROUP_XOR] = "XOR",
    [GROUP_IF] = "IF",
    [GROUP_LESS] = "LESS",
    [GROUP_LESS_EQUAL] = "LESS_EQUAL",
    [GROUP_GREATER] = "GREATER",
    [GROUP_GREATER_EQUAL] = "GREATER_EQUAL",
    [GROUP_EQUAL] = "EQUAL",
    [GROUP_NOT_EQUAL] = "NOT_EQUAL",
    [GROUP_COMMA] = "COMMA",
    [GROUP_SEMICOLON] = "SEMICOLON",
    [GROUP_DO] = "DO",
    [GROUP_WHERE] = "WHERE",
    [GROUP_EXCLAM] = "EXCLAM",
    [GROUP_PERCENT] = "PERCENT",
    [GROUP_ELSE] = "ELSE",
    [GROUP_ROUND] = "ROUND",
    [GROUP_DOUBLE_CORNER] = "DOUBLE_CORNER",
    [GROUP_CORNER] = "CORNER",
    [GROUP_SQUARE] = "SQUARE",
    [GROUP_CURLY] = "CURLY",
    [GROUP_DOUBLE_BAR] = "DOUBLE_BAR",
    [GROUP_BAR] = "BAR",
    [GROUP_CEIL] = "CEIL",
    [GROUP_FLOOR] = "FLOOR",
    [GROUP_IMPLICIT] = "IMPLICIT",
    [GROUP_VARIABLE] = "VARIABLE",
    [GROUP_NUMBER] = "NUMBER",
    [GROUP_ELEMENT_OF] = "ELEMENT_OF",
    [GROUP_RAISE2] = "RAISE2",
    [GROUP_RAISE3] = "RAISE3",
    [GROUP_SQRT] = "SQRT",
    [GROUP_CBRT] = "CBRT",
};

const char *GroupTypeOperatorStrings[] = {
    [GROUP_NULL] = ".",
    [GROUP_POSITIVE] = "+.",
    [GROUP_NEGATE] = "-.",
    [GROUP_NOT] = "!.",
    [GROUP_PLUS] = ". + .",
    [GROUP_MINUS] = ". - .",
    [GROUP_MULTIPLY] = ". * .",
    [GROUP_DIVIDE] = ". / .",
    [GROUP_MOD] = ". mod .",
    [GROUP_RAISE] = ".^.",
    [GROUP_LOWER] = "._.",
    [GROUP_AND] = ". and .",
    [GROUP_OR] = ". or .",
    [GROUP_XOR] = ". xor .",
    [GROUP_IF] = ". if .",
    [GROUP_LESS] = ". < .",
    [GROUP_LESS_EQUAL] = ". ≤ .",
    [GROUP_GREATER] = ". > .",
    [GROUP_GREATER_EQUAL] = ". ≥ .",
    [GROUP_EQUAL] = ". = .",
    [GROUP_NOT_EQUAL] = ". ≠ .",
    [GROUP_COMMA] = "., .",
    [GROUP_SEMICOLON] = ".; .",
    [GROUP_DO] = ". do .",
    [GROUP_WHERE] = ". where .",
    [GROUP_EXCLAM] = ".!",
    [GROUP_PERCENT] = ".%",
    [GROUP_ELSE] = ". else",
    [GROUP_ROUND] = "(.)",
    [GROUP_DOUBLE_CORNER] = "<<.>>",
    [GROUP_CORNER] = "<.>",
    [GROUP_SQUARE] = "[.]",
    [GROUP_CURLY] = "{.}",
    [GROUP_DOUBLE_BAR] = "‖.‖",
    [GROUP_BAR] = "|.|",
    [GROUP_CEIL] = "⌈.⌉",
    [GROUP_FLOOR] = "⌊.⌋",
    [GROUP_IMPLICIT] = "..",
    [GROUP_VARIABLE] = "",
    [GROUP_NUMBER] = "",
    [GROUP_ELEMENT_OF] = ". ∈ .",
    [GROUP_RAISE2] = ".²",
    [GROUP_RAISE3] = ".³",
    [GROUP_SQRT] = "√.",
    [GROUP_CBRT] = "∛.",
};

void output_group_debug(const struct group *g, int color)
{
    if (color == 0) {
        color = 37;
    } else {
        color = color + 1;
        if (color == 38) {
            color = 31;
        }
    }
    printf("\x1b[1;%dm", color);
    switch (g->t) {
    case GROUP_NUMBER:
        mpf_out_str(stdout, 10, 0, g->v.f);
        break;
    case GROUP_VARIABLE:
        printf("%s", g->v.w);
        break;
    default:
        printf("%s", GroupTypeStrings[g->t]);
    }
    if (g->n > 0) {
        printf("[\x1b[m");
        for (size_t i = 0; i < g->n; i++) {
            if (i > 0) {
                printf(", ");
            }
            output_group_debug(&g->g[i], color);
        }
        printf("\x1b[1;%dm]", color);
    }
    printf("\x1b[m");
}

void output_group(const struct group *g)
{
    switch (g->t) {
    case GROUP_NUMBER:
        mpf_out_str(stdout, 10, 0, g->v.f);
        return;
    case GROUP_VARIABLE:
        printf("%s", g->v.w);
        return;
    default:
        break;
    }
    const char *s = GroupTypeOperatorStrings[g->t];
    const char *d = strchr(s, '.');
    for (size_t i = 0; i < g->n; i++) {
        printf("%.*s", (int) (d - s), s);
        output_group(&g->g[i]);
        s = d + 1;
        d = strchr(s, '.');
        if (d == NULL && i + 1 != g->n) {
            fprintf(stderr, "internal error of eternal weirdness\n");
            exit(1);
        }
    }
    printf("%s", s);
}
