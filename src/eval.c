#include "core.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int pass_single(struct value *v, struct value *values)
{
    copy_value(v, &values[0]);
    return 0;
}

static int operate(struct value *v, struct value *values, size_t n, enum group_type opr);

#include "eval_number.h"
#include "eval_bool.h"
#include "eval_matrix.h"

static int operate(struct value *v, struct value *values, size_t n, enum group_type opr)
{
    static int (*single_operators[GROUP_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_POSITIVE][VALUE_NUMBER] = pass_single,
        [GROUP_POSITIVE][VALUE_MATRIX] = pass_single,

        [GROUP_NEGATE][VALUE_NUMBER] = number_negate,
        [GROUP_NOT][VALUE_NUMBER] = bool_not,

        [GROUP_ROUND][VALUE_BOOL] = pass_single,
        [GROUP_ROUND][VALUE_NUMBER] = pass_single,
        [GROUP_ROUND][VALUE_MATRIX] = pass_single,
        [GROUP_ROUND][VALUE_SET] = pass_single,
        [GROUP_ROUND][VALUE_RANGE] = pass_single,
    };

    static int (*double_operators[GROUP_MAX][VALUE_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_COMMA][VALUE_NUMBER][VALUE_NUMBER] = number_comma_number,
        [GROUP_COMMA][VALUE_NUMBER][VALUE_MATRIX] = number_comma_matrix,
        [GROUP_COMMA][VALUE_MATRIX][VALUE_NUMBER] = matrix_comma_number,
        [GROUP_COMMA][VALUE_MATRIX][VALUE_MATRIX] = matrix_comma_matrix,

        [GROUP_SEMICOLON][VALUE_MATRIX][VALUE_MATRIX] = matrix_semicolon_matrix,

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

int copy_value(struct value *dest, const struct value *src)
{
    dest->t = src->t;
    switch (src->t) {
    case VALUE_NULL:
        return -1;
    case VALUE_BOOL:
        dest->v.b = src->v.b;
        break;
    case VALUE_NUMBER:
        mpf_init_set(dest->v.f, src->v.f);
        break;
    case VALUE_MATRIX: {
        const struct matrix *mat = &src->v.m;
        const size_t n = mat->n * mat->m;
        dest->v.m.v = reallocarray(NULL, n, sizeof(*mat->v));
        for (size_t i = 0; i < n; i++) {
            copy_value(&dest->v.m.v[i], &mat->v[i]);
        }
        dest->v.m.n = mat->n;
        dest->v.m.m = mat->m;
        break;
    }
    case VALUE_RANGE:
        /* TODO: */
        break;
    case VALUE_SET:
        /* TODO: */
        break;
    case VALUE_MAX:
        return -1;
    }
    return 0;
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

/* TODO: consider making this recursive again */
int compute_value(struct group *g, struct value *v)
{
    struct value r[128]; /* TODO: what size should this be, dynamic? */
    int nr = 0;
    struct variable *var;

    while (1) {
        switch (g->t) {
        case GROUP_NUMBER:
            r[nr].t = VALUE_NUMBER;
            mpf_init_set(r[nr].v.f, g->v.f);
            nr++;
            break;
        case GROUP_VARIABLE:
            var = get_variable(g);
            if (var == NULL) {
                throw_error("variable does not exist");
                return -1;
            }
            compute_value(&var->value, &r[nr++]);
            break;
        case GROUP_EQUAL:
            if (nr == 0) {
                if (g->g[0].t == GROUP_VARIABLE) {
                    var = get_variable(&g->g[0]);
                    if (var == NULL) {
                        add_variable(&g->g[0], &g->g[1]);
                    } else {
                        clear_group(&var->value);
                        copy_group(&var->value, &g->g[1]);
                    }
                    break;
                }
            }
            /* fall through */
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
            for (size_t i = 0; i < g->p->n; i++) {
                clear_value(&r[--nr]);
            }
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
            printf("\n");
            for (size_t i = 0; i < v->v.m.m; i++) {
                for (size_t j = 0; j < v->v.m.n; j++) {
                    output_value(&v->v.m.v[i * v->v.m.n + j]);
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
