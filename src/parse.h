#ifndef PARSE_H
#define PARSE_H

#include <setjmp.h>
#include <stdio.h>
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

    jmp_buf jb;
} Parser;

enum group_type {
    GROUP_NULL,

    /* opr expr */
    GROUP_POSITIVE,
    GROUP_NEGATE,
    GROUP_NOT,

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
    GROUP_LESS_THAN,
    GROUP_GREATER,
    GROUP_GREATER_THAN,
    GROUP_EQUALS,
    GROUP_NOT_EQUALS,

    GROUP_COMMA,
    GROUP_SEMICOLON,

    GROUP_DO,
    GROUP_WHERE,

    /* expr opr */
    GROUP_EXCLAM,
    GROUP_PERCENT,
    GROUP_ELSE,

    /* opr expr opr */
    GROUP_ROUND,
    GROUP_DOUBLE_CORNER,
    GROUP_CORNER,
    GROUP_SQUARE,
    GROUP_CURLY,
    GROUP_DOUBLE_BAR,
    GROUP_BAR,

    /* opr expr opr expr */
    GROUP_CHOOSE_FROM,

    /* expr expr */
    GROUP_IMPLICIT,

    GROUP_VARIABLE,
    GROUP_NUMBER,
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
struct group *join_group(struct group *parent, struct group *child);
struct group *join_group_no_free(struct group *parent, struct group *child);
struct group *surround_group(struct group *group, enum group_type type);
void clear_group(struct group *g);
void free_group(struct group *g);
void output_group(struct group *g, int color);

void throw_error(const char *msg, ...);
int parse(struct group *group);

#endif
