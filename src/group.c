#include "parse.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct group *new_group(size_t n)
{
    struct group *g;

    g = calloc(n, sizeof(*g));
    if (g == NULL) {
        throw_error("%s", strerror(errno));
        return NULL;
    }
    return g;
}

struct group *join_group(struct group *p, struct group *c)
{
    join_group_no_free(p, c);
    free(c);
    return p;
}

struct group *join_group_no_free(struct group *p, struct group *c)
{
    struct group *g;

    g = reallocarray(p->g, p->n + 1, sizeof(*p->g));
    if (g == NULL) {
        throw_error("%s", strerror(errno));
        return NULL;
    }
    p->g = g;
    g[p->n++] = *c;
    return p;
}

struct group *surround_group(struct group *c, enum group_type t, size_t n)
{
    struct group *g;

    g = new_group(n);
    if (g == NULL) {
        return NULL;
    }
    g[0] = *c;
    c->t = t;
    c->g = g;
    c->n = n;
    return &g[n - 1];
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
    [GROUP_LESS_THAN] = "LESS_THAN",
    [GROUP_GREATER] = "GREATER",
    [GROUP_GREATER_THAN] = "GREATER_THAN",
    [GROUP_EQUALS] = "EQUALS",
    [GROUP_NOT_EQUALS] = "NOT_EQUALS",
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
    [GROUP_IMPLICIT] = "IMPLICIT",
    [GROUP_VARIABLE] = "VARIABLE",
    [GROUP_NUMBER] = "NUMBER",
    [GROUP_ELEMENT_OF] = "ELEMENT_OF",
    [GROUP_RAISE2] = "RAISE2",
    [GROUP_RAISE3] = "RAISE3",
    [GROUP_SQRT] = "SQRT",
    [GROUP_CBRT] = "CBRT",
};

void output_group(struct group *g, int color)
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
            output_group(&g->g[i], color);
        }
        printf("\x1b[1;%dm]", color);
    }
    printf("\x1b[m");
}
