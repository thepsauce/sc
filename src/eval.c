#include "core.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

void noop(struct value *v, struct value *values)
{
    (void) v;
    (void) values;
    /* does nothing */
}

static void operate(struct value *v, struct value *values, size_t n, enum group_type opr);

#include "eval_number.h"
#include "eval_bool.h"
#include "eval_matrix.h"

static void operate(struct value *v, struct value *values, size_t n, enum group_type opr)
{
    static void (*single_operators[GROUP_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_POSITIVE][VALUE_NUMBER] = noop,
        [GROUP_POSITIVE][VALUE_MATRIX] = noop,

        [GROUP_NEGATE][VALUE_NUMBER] = number_negate,
        [GROUP_NOT][VALUE_NUMBER] = bool_not,
    };

    static void (*double_operators[GROUP_MAX][VALUE_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_PLUS][VALUE_NUMBER][VALUE_NUMBER] = number_plus_number,
        [GROUP_MINUS][VALUE_NUMBER][VALUE_NUMBER] = number_minus_number,
        [GROUP_MULTIPLY][VALUE_NUMBER][VALUE_NUMBER] = number_multiply_number,
        [GROUP_DIVIDE][VALUE_NUMBER][VALUE_NUMBER] = number_divide_number,

        [GROUP_MULTIPLY][VALUE_NUMBER][VALUE_MATRIX] = number_multiply_matrix,
        [GROUP_MULTIPLY][VALUE_MATRIX][VALUE_NUMBER] = matrix_multiply_number,

        [GROUP_MULTIPLY][VALUE_MATRIX][VALUE_MATRIX] = matrix_multiply_matrix,

        [GROUP_PLUS][VALUE_MATRIX][VALUE_MATRIX] = matrix_plus_matrix,
        [GROUP_MINUS][VALUE_MATRIX][VALUE_MATRIX] = matrix_minus_matrix,

        [GROUP_AND][VALUE_BOOL][VALUE_BOOL] = bool_and_bool,
        [GROUP_OR][VALUE_BOOL][VALUE_BOOL] = bool_or_bool,
        [GROUP_XOR][VALUE_BOOL][VALUE_BOOL] = bool_xor_bool,

        [GROUP_LESS][VALUE_NUMBER][VALUE_NUMBER] = number_less_number,
        [GROUP_LESS_EQUAL][VALUE_NUMBER][VALUE_NUMBER] = number_less_equal_number,
        [GROUP_GREATER][VALUE_NUMBER][VALUE_NUMBER] = number_greater_number,
        [GROUP_GREATER_EQUAL][VALUE_NUMBER][VALUE_NUMBER] = number_greater_equal_number,
        [GROUP_EQUAL][VALUE_NUMBER][VALUE_NUMBER] = number_equal_number,
        [GROUP_NOT_EQUAL][VALUE_NUMBER][VALUE_NUMBER] = number_not_equal_number,
    };

    void (*op)(struct value *v, struct value *vs);

    switch (n) {
    case 1:
        op = single_operators[opr][values[0].t];
        break;
    case 2:
        op = double_operators[opr][values[0].t][values[1].t];
        break;
    default:
        op = NULL;
    }

    if (op == NULL) {
        throw_error("operator not defined");
    }
    (*op)(v, values);
}

void clear_value(struct value *v)
{
    switch (v->t) {
    case VALUE_NUMBER:
        mpf_clear(v->v.f);
        break;
    case VALUE_MATRIX:
        for (size_t i = 0, n = v->v.m.m * v->v.m.n;
                i < n; i++) {
            clear_value(&v->v.m.v[i]);
        }
        free(v->v.m.v);
        break;
    case VALUE_SET:
        break;
    case VALUE_RANGE:
        break;
    default:
        break;
    }
}

void compute_deeper_value(const struct group *g, struct value *v)
{
    struct variable *var;
    bool cw;

beg:
    switch (g->t) {
    case GROUP_NUMBER:
        v->t = VALUE_NUMBER;
        mpf_init_set(v->v.f, g->v.f);
        break;
    case GROUP_VARIABLE:
    case GROUP_LOWER:
        var = get_variable(g);
        if (var == NULL) {
            throw_error("variable '%s' does not exist", g->v.w);
        }
        break;
    case GROUP_DOUBLE_CORNER:
        if (g->g[0].t == GROUP_COMMA) {
            struct value values[2];
            for (size_t i = 0; i < g->n; i++) {
                compute_deeper_value(&g->g[i], &values[i]);
            }
            vector_dot_product(v, values);
            for (size_t i = 0; i < g->n; i++) {
                clear_value(&values[i]);
            }
        }
        break;
    case GROUP_COMMA:
        if (Core.w.m == 0) {
            Core.w.m = 1;
            cw = true;
        } else {
            cw = false;
        }
        compute_deeper_value(&g->g[0], v);
        if (v->t != VALUE_MATRIX) {
            Core.w.n++;
            Core.w.v = reallocarray(Core.w.v, Core.w.n * Core.w.m, sizeof(*Core.w.v));
            Core.w.v[Core.w.i++] = *v;
        }
        compute_deeper_value(&g->g[1], v);
        if (v->t != VALUE_MATRIX) {
            Core.w.n++;
            Core.w.v = reallocarray(Core.w.v, Core.w.n * Core.w.m, sizeof(*Core.w.v));
            Core.w.v[Core.w.i++] = *v;
        }
        v->t = VALUE_MATRIX;
        if (cw) {
            v->v.m.v = Core.w.v;
            v->v.m.m = Core.w.m;
            v->v.m.n = Core.w.n;
            Core.w.v = NULL;
            Core.w.m = 0;
            Core.w.n = 0;
            Core.w.i = 0;
        }
        break;
    case GROUP_ROUND:
        switch (g->g[0].t) {
        case GROUP_SEMICOLON:
            compute_deeper_value(&g->g[0].g[0], v);
            break;
        default:
            g = &g->g[0];
            goto beg;
        }
        break;
    default: {
        struct value values[g->n];
        for (size_t i = 0; i < g->n; i++) {
            compute_deeper_value(&g->g[i], &values[i]);
        }
        operate(v, values, g->n, g->t);
        for (size_t i = 0; i < g->n; i++) {
            clear_value(&values[i]);
        }
    }
    }
}

int compute_value(const struct group *g, struct value *v)
{
    struct variable *var;

    if (setjmp(Core.jb) != 0) {
        return -1;
    }
    switch (g->t) {
    case GROUP_EQUAL:
        var = get_variable(g->g);
        if (g->g->t == GROUP_IMPLICIT) {
            /* function declaration */
            break;
        }
        if (g->g->t == GROUP_VARIABLE) {
            if (var == NULL) {
                add_variable(&g->g[0], &g->g[1]);
            } else {
                copy_group(&var->value, &g->g[1]);
            }
            compute_deeper_value(&g->g[1], v);
            break;
        }
    /* fall through */
    default:
        compute_deeper_value(g, v);
    }
    return 0;
}

void output_value(struct value *v)
{
    switch (v->t) {
    case VALUE_BOOL:
        printf("%s", v->v.b ? "true" : "false");
        break;
    case VALUE_NUMBER:
        mpf_out_str(stdout, 10, 0, v->v.f);
        break;
    case VALUE_MATRIX:
        if (v->v.m.m == 1) {
            printf("( ");
            for (size_t i = 0; i < v->v.m.n; i++) {
                output_value(&v->v.m.v[i]);
                printf(" ");
            }
            printf(")");
        } else {
            for (size_t i = 0; i < v->v.m.n; i++) {
                for (size_t j = 0; j < v->v.m.m; j++) {
                    output_value(&v->v.m.v[i + j * v->v.m.n]);
                    printf(" ");
                }
                printf("\n");
            }
        }
        break;
    case VALUE_SET:
        break;
    case VALUE_RANGE:
        break;
    default:
        break;
    }
}
