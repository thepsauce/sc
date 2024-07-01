#ifndef GROUP_H
#define GROUP_H

#include <stdio.h>

#include <mpfr.h>

#include <math.h>

enum group_type {
    GROUP_NULL,

    /* opr expr */
    GROUP_POSITIVE,
    GROUP_NEGATE,
    GROUP_NOT,
    GROUP_SQRT,
    GROUP_CBRT,

    /* expr opr expr */
    GROUP_PLUS,
    GROUP_MINUS,
    GROUP_MULTIPLY,
    GROUP_DIVIDE,
    GROUP_MOD,
    GROUP_RAISE,
    GROUP_LOWER,

    GROUP_AND,
    GROUP_OR,
    GROUP_XOR,

    GROUP_IF,

    GROUP_LESS,
    GROUP_LESS_EQUAL,
    GROUP_GREATER,
    GROUP_GREATER_EQUAL,
    GROUP_EQUAL,
    GROUP_NOT_EQUAL,

    GROUP_COMMA,
    GROUP_SEMICOLON,

    GROUP_DO,
    GROUP_WHERE,

    GROUP_ELEMENT_OF,

    /* expr opr */
    GROUP_EXCLAM,
    GROUP_PERCENT,
    GROUP_RAISE2,
    GROUP_RAISE3,
    GROUP_ELSE,

    /* opr expr opr */
    GROUP_ROUND,
    GROUP_DOUBLE_CORNER,
    GROUP_CORNER,
    GROUP_SQUARE,
    GROUP_CURLY,
    GROUP_DOUBLE_BAR,
    GROUP_BAR,
    GROUP_CEIL,
    GROUP_FLOOR,

    /* expr expr */
    GROUP_IMPLICIT,

    GROUP_VARIABLE,
    GROUP_NUMBER,

    GROUP_MAX
};

struct group {
    enum group_type t;
    struct group *g;
    size_t n;
    union {
        char *w;
        mpf_t f;
    } v;
};

struct group *new_group(size_t num);
int copy_group(struct group *dest, const struct group *src);
struct group *join_group(struct group *parent, struct group *child);
struct group *join_group_no_free(struct group *parent, struct group *child);
struct group *surround_group(struct group *group, enum group_type type, size_t n);
void clear_group(struct group *group);
void free_group(struct group *group);
void output_group(struct group *group, int color);

#endif

