#include "core.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

int noop(struct value *v, struct value *values)
{
    (void) v;
    (void) values;
    /* does nothing */
    return 0;
}

static int operate(struct value *v, struct value *values, size_t n, enum group_type opr);

#include "eval_number.h"
#include "eval_bool.h"
#include "eval_matrix.h"

static int operate(struct value *v, struct value *values, size_t n, enum group_type opr)
{
    static int (*single_operators[GROUP_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_POSITIVE][VALUE_NUMBER] = noop,
        [GROUP_POSITIVE][VALUE_MATRIX] = noop,

        [GROUP_NEGATE][VALUE_NUMBER] = number_negate,
        [GROUP_NOT][VALUE_NUMBER] = bool_not,
    };

    static int (*double_operators[GROUP_MAX][VALUE_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
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

    int (*op)(struct value *v, struct value *vs);

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
        return -1;
    }
    return (*op)(v, values);
}

void clear_value(struct value *v)
{
    switch (v->t) {
    case VALUE_NUMBER:
        mpf_clear(v->v.f);
        break;
    case VALUE_MATRIX:
        for (size_t i = 0, n = v->v.m.m * v->v.m.n; i < n; i++) {
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

int compute_value(struct group *g, struct value *v)
{
    struct value r[128]; /* TODO: what size should this be, dynamic? */
    int nr = 0;

    while (1) {
        switch (g->t) {
        case GROUP_NUMBER:
            r[nr].t = VALUE_NUMBER;
            mpf_init_set(r[nr].v.f, g->v.f);
            nr++;
            break;
        case GROUP_VARIABLE:
            throw_error("no variables right now");
            return -1;
        default /* some operator */:
            g = g->g;
            continue;
        }
        while (g->p == NULL || g + 1 == g->p->g + g->p->n) {
            if (g->p == NULL) {
                *v = r[0];
                return 0;
            }
            if (operate(v, &r[nr - g->p->n], g->p->n, g->p->t) == -1) {
                return -1;
            }
            nr -= g->p->n;
            r[nr++] = *v;
            g = g->p;
        }
        g++;
    }
    /* this point is never reached */
}

void output_value(struct value *v)
{
    switch (v->t) {
    case VALUE_NULL:
        /* nothing, but problem.. */
        break;
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
        /* TODO: */
        break;
    case VALUE_RANGE:
        /* TODO: */
        break;
    case VALUE_MAX:
        /* nothing */
        break;
    }
}
