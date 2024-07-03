#ifndef CORE_H
#define CORE_H

#include "group.h"

#include <stdbool.h>
#include <setjmp.h>

/* * * Value * * */

/*
 * Values are computed groups that store data depending on which
 * type (enum value_type) they are, see below for the available types.
 */
enum value_type {
    VALUE_NULL,
    VALUE_BOOL,
    VALUE_NUMBER,
    /* VALUE_VECTOR, (matrix of size 1 x n) */
    VALUE_MATRIX,
    VALUE_SET,
    VALUE_RANGE,
    VALUE_MAX
};

struct value;

struct matrix {
    /* data */
    struct value *v;
    /* m: rows,
     * n: columns
     */
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

struct variable {
    /* the name can be of type GROUP_VARIABLE, GROUP_LOWER or GROUP_IMPLICIT */
    struct group name;
    struct group value;
};

/* * * Core * * */

extern struct core {
    /* all variables */
    struct variable *v;
    size_t nv;
} Core;

/*
 * Computes the value of the group.
 */
int compute_value(/* const */ struct group *group, struct value *value);

/*
 * Frees resources associated with this value.
 */
void clear_value(struct value *value);

/*
 * Outputs the value neatly into stdout.
 */
void output_value(struct value *value);

/*
 * Use a group (see above for possible types) to get the variable.
 * This function can be used to set a variable or to get its values.
 */
struct variable *get_variable(const struct group *name);

/*
 * Simply adds the named value to the variable list.
 *
 * Note: This does not check if the variable exists already
 */
int add_variable(const struct group *name, const struct group *val);

/*
 * Print the error to stderr.
 */
void throw_error(const char *fmt, ...);

#endif

