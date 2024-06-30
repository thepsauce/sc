#ifndef CORE_H
#define CORE_H

#include "group.h"

#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>

#include <setjmp.h>

enum value_type {
    VALUE_BOOL,
    VALUE_NUMBER,
    /* VALUE_VECTOR, (matrix of size 1 x n) */
    VALUE_MATRIX,
    VALUE_SET,
    VALUE_RANGE,
    VALUE_MAX
};

struct value;

struct vector {
    struct value *v;
    size_t n;
};

struct matrix {
    struct value *v;
    size_t m, n;
};

struct value {
    enum value_type t;
    union {
        mpf_t f;
        bool b;
        struct matrix /* vector */ v;
        struct matrix m;
        struct matrix /* set */ s;
    } v;
};

int compute_value(const struct group *group, struct value *value);
void clear_value(struct value *value);
void output_value(struct value *value);

struct variable {
    /* the name can be of type GROUP_VARIABLE, GROUP_LOWER or GROUP_IMPLICIT */
    struct group name;
    struct group value;
};

extern struct core {
    /* all variables */
    struct variable *v;
    size_t nv;

    struct working_matrix {
        struct value *v;
        size_t m, n;
        size_t i;
    } w;
    /* set when calling compute_value and used
     * for throw_error */
    jmp_buf jb;
} Core;

struct variable *get_variable(const struct group *g);
void add_variable(const struct group *name, const struct group *val);
void throw_error(const char *fmt, ...);

#endif

