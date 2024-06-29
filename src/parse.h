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
    int n;

    /* stack */
    struct group *st[1024];
    /* stack pointer */
    int sp;
    /* open brackets */
    int brackets[64];
    /* open bracket pointer */
    int bp;
} Parser;

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
    GROUP_LESS_THAN,
    GROUP_GREATER,
    GROUP_GREATER_THAN,
    GROUP_EQUALS,
    GROUP_NOT_EQUALS,

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
struct group *surround_group(struct group *group, enum group_type type, size_t n);
void clear_group(struct group *group);
void free_group(struct group *group);
void output_group(struct group *group, int color);

enum value_type {
    VALUE_NUMBER,
    VALUE_VECTOR,
    VALUE_MATRIX,
    VALUE_SET,
    VALUE_RANGE
};

struct value {
    enum value_type t;
    union {
        mpf_t f;
    } v;
};

void compute_value(const struct group *group, struct value *value);
void clear_value(struct value *value);
void output_value(struct value *value);

void throw_error(const char *msg, ...);

#define PARSER_ERROR (-1)
#define PARSER_OK (0)
#define PARSER_CONTINUE (1)

int init_parser(void);
void reset_parser(void);
int parse(const char *s);

#endif
