#ifndef SC_H
#define SC_H

#include <stdio.h>
#include <gmp.h>
#include <mpfr.h>

struct expression;

typedef struct number {
    mpf_t value;
} Number;

typedef struct range {
    struct expression *start;
    struct expression *end;
} Range;

typedef struct set {
    struct expression **members;
    size_t numMembers;
} Set;

typedef struct variable {
    const char *name;
    struct expression *value;
} Variable;

int put_variable(Variable *var);
int get_variable(const char *name, size_t lenName, Variable *var);

typedef struct function_data {
    char **args;
    size_t numArgs;
    struct expression *returnValue;
} FunctionData;

/* functions can have the same name but different argument count and return
 * value */
typedef struct function_set {
    char *name;
    size_t numData;
    FunctionData *data;
} FunctionSet;

typedef struct function {
    char *name;
    FunctionData data;
} Function;

int put_function(Function *func);
int get_function(const char *name, size_t lenName, FunctionSet *funcs);

typedef struct vector {
    struct expression **components;
    size_t numComponents;
} Vector;

typedef struct matrix {
    struct expression **components;
    size_t numRows, numCols;
} Matrix;

enum expression_type {
    EXPR_NULL,

    /* Double operators */
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_POW,
    EXPR_EQUAL,
    EXPR_ELEMENT_OF,
    EXPR_AND,
    EXPR_MOD,
    EXPR_WHERE,
    EXPR_CHOOSE,
    EXPR_EXT,
    EXPR_BAR,

    /* Single operators */
    EXPR_AROUND_BAR,
    EXPR_AROUND_DOUBLE_BAR,
    EXPR_NEGATE,
    EXPR_FACTORIAL,

    /* Variables */
    EXPR_VARIABLE,
    EXPR_FUNCTION_CALL,

    /* Primitives */
    EXPR_NUMBER,
    EXPR_VECTOR,
    EXPR_MATRIX,
    EXPR_SET,
    EXPR_RANGE,

    EXPR_MAX,
};

enum expression_meta_type {
    EXPR_META_NULL,

    EXPR_META_L, /* operator on the left of an expression */
    EXPR_META_R, /* operator is on the right of an expression */
    EXPR_META_S, /* operator surrounds and expression */
    EXPR_META_M, /* operator is in the middle of two expressions */

    EXPR_META_VARIABLE = EXPR_VARIABLE,
    EXPR_META_FUNCTION_CALL,

    EXPR_META_NUMBER,
    EXPR_META_VECTOR,
    EXPR_META_MATRIX,
    EXPR_META_SET,
    EXPR_META_RANGE,
};

struct expression_meta {
    enum expression_meta_type type;
    enum expression_type exprType;
    unsigned precedence;
    const char *sequence;
};

const struct expression_meta *get_meta(enum expression_type type);
const struct expression_meta *get_meta_sequence(const char *seq);

typedef struct expression {
    enum expression_type type;
    union {
        /* operator with one input */
        struct expression *expr;
        /* operator  with two inputs */
        struct {
            struct expression *left;
            struct expression *right;
        } duo;
        Number number;
        Vector vector;
        Matrix matrix;
        Set set;
        Range range;
        /* variable access */
        const char *name;
        /* function call */
        struct {
            const char *name;
            struct expression **args;
            size_t numArgs;
        } call;
    } value;
} Expression;

int output_expression(const Expression *expr, FILE *fp);
Expression *input_expression(FILE *fp);
Expression *simplify_expression(Expression *expr);
void delete_expression(Expression *expr);

#endif
