#ifndef CORE_H
#define CORE_H

#include "group.h"

#include <stdio.h>
#include <gmp.h>

#include <setjmp.h>

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

int compute_value(const struct group *group, struct value *value);
void clear_value(struct value *value);
void output_value(struct value *value);

struct variable {
    /* the name can be of type GROUP_VARIABLE, GROUP_LOWER or GROUP_IMPLICIT */
    struct group name;
    struct group value;
};

extern struct core {
    struct variable *v;
    size_t nv;
    jmp_buf jb;
} Core;

struct variable *get_variable(const struct group *g);
void add_variable(const struct group *name, const struct group *val);
void throw_error(const char *fmt, ...);

#endif

