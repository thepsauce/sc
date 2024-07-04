#ifndef GROUP_H
#define GROUP_H

#include <stdio.h>

#include <mpfr.h>

#include <math.h>

/* * * Group * * */
/* The life cycle of a group is:
 * 0. The group is created (new_group())
 * 1. The parser parses a string and generates groups (parse()).
 * 2.1 The core evaluates this group (compute_value()).
 * 2.2 Or the group is stored in a variable (compute_value() as well)
 * 3.1 The group is destroyed after no longer needed (free_group()).
 * 3.2 Or the address of the group is reused (clear_group()).
 *
 * Below are all types..
 */

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

extern const int Precedences[];

struct group {
    /* type of this group */
    enum group_type t;
    /* children of the group */
    struct group *g;
    size_t n;
    /* parent of the group */
    struct group *p;
    union {
        /* GROUP_VARIABLE */
        char *w;
        /* GROUP_NUMBER */
        mpf_t f;
    } v;
};

/*
 * Uses calloc() to create a pointer to num groups.
 * Use this to create a single group (group_new(1)) or
 * the children of a group (group_new(n)).
 */
struct group *new_group(size_t num);

/*
 * Creates a deep copy of src and stores it into dest.
 */
int copy_group(struct group *dest, const struct group *src);

/*
 * Make the group a parent group with n children, the first child will be the
 * group itself and the parent will have type type.
 */
struct group *surround_group(struct group *group, enum group_type type, size_t n);

/*
 * Cleared the memory used by a group, this does not free the group pointer.
 */
void clear_group(struct group *group);

/*
 * Same as clear_group() but it also uses free(group).
 */
void free_group(struct group *group);

/*
 * Output this group to stdout (use 0 for color)
 */
void output_group_debug(const struct group *group, int color);
void output_group(const struct group *group);

#endif

