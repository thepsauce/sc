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
    char *name;
    struct group value;
    /* dependencies */
    char **dep;
    size_t ndep;
};

/* * * Core * * */

extern struct core {
    /* all variables (this needs to be a double pointer,
     * so the address of value does not change (messes up the children))
     */
    struct variable **v;
    size_t nv;
} Core;

/*
 * Computes the value of the group.
 */
int compute_value(/* const */ struct group *group, struct value *value);

/*
 * Copies src into dest, allocates new memory so they are independent.
 */
int copy_value(struct value *dest, const struct value *src);

/*
 * Frees resources associated with this value.
 */
void clear_value(struct value *value);

/*
 * Outputs the value neatly into stdout.
 */
void output_value(struct value *value);

/*
 * Get a variable by name, also with number of arguments to allow for example:
 * f(x) = x
 * f(x, y) = x + y
 *
 * This function can be used to set a variable or to get its values.
 *
 * Note: The name must be contained in the dictionary (added with dict_put).
 */
struct variable *get_variable(const char *name, size_t ndep);

/*
 * Simply adds the named value to the variable list.
 *
 * Note: This does not check if the variable exists already. The name must
 * be contained in the dictionary.
 */
int add_variable(char *name, const struct group *val, char *const *dep, size_t ndep);

/*
 * Print the error to stderr.
 */
void throw_error(const char *fmt, ...);

#endif

