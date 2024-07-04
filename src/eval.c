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
        [GROUP_COMMA][VALUE_BOOL][VALUE_NUMBER] = value_comma_value,
        [GROUP_COMMA][VALUE_BOOL][VALUE_MATRIX] = value_comma_matrix,
        [GROUP_COMMA][VALUE_MATRIX][VALUE_BOOL] = matrix_comma_value,
        [GROUP_COMMA][VALUE_NUMBER][VALUE_NUMBER] = value_comma_value,
        [GROUP_COMMA][VALUE_NUMBER][VALUE_MATRIX] = value_comma_matrix,
        [GROUP_COMMA][VALUE_MATRIX][VALUE_NUMBER] = matrix_comma_value,
        [GROUP_COMMA][VALUE_MATRIX][VALUE_MATRIX] = matrix_comma_matrix,

        [GROUP_SEMICOLON][VALUE_BOOL][VALUE_NUMBER] = value_semicolon_value,
        [GROUP_SEMICOLON][VALUE_BOOL][VALUE_MATRIX] = value_semicolon_matrix,
        [GROUP_SEMICOLON][VALUE_MATRIX][VALUE_BOOL] = matrix_semicolon_value,
        [GROUP_SEMICOLON][VALUE_NUMBER][VALUE_NUMBER] = value_semicolon_value,
        [GROUP_SEMICOLON][VALUE_NUMBER][VALUE_MATRIX] = value_semicolon_matrix,
        [GROUP_SEMICOLON][VALUE_MATRIX][VALUE_NUMBER] = matrix_semicolon_value,
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
        const struct matrix *mat = &src->v.mat;
        const size_t n = mat->n * mat->m;
        dest->v.mat.v = reallocarray(NULL, n, sizeof(*mat->v));
        for (size_t i = 0; i < n; i++) {
            copy_value(&dest->v.mat.v[i], &mat->v[i]);
        }
        dest->v.mat.n = mat->n;
        dest->v.mat.m = mat->m;
        break;
    }

    case VALUE_VARIABLE:
        dest->v.var = src->v.var;
        break;
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
        for (size_t i = 0, n = v->v.mat.m * v->v.mat.n; i < n; i++) {
            clear_value(&v->v.mat.v[i]);
        }
        free(v->v.mat.v);
        break;
    case VALUE_SET:
        break;
    case VALUE_RANGE:
        break;
    default:
        break;
    }
}

static int compute_deep_value(struct group *g, struct value *v)
{
    struct variable *var;

    switch (g->t) {
    case GROUP_NUMBER:
        v->t = VALUE_NUMBER;
        mpf_init_set(v->v.f, g->v.f);
        return 0;

    case GROUP_VARIABLE:
        var = get_variable(g->v.w, 0);
        if (var == NULL) {
            throw_error("variable does not exist");
            return -1;
        }
        return compute_deep_value(&var->value, v);

    case GROUP_IMPLICIT:
        if (g->g[0].t == GROUP_VARIABLE) {
            /* TODO: */
        }
        throw_error("implicit operations not implemented yet");
        return -1;

    default:
        break;
    }

    struct value values[g->n];
    for (size_t i = 0; i < g->n; i++) {
        if (compute_deep_value(&g->g[i], &values[i]) == -1) {
            while (i > 0) {
                clear_value(&values[--i]);
            }
            return -1;
        }
    }
    const int err = operate(v, values, g->n, g->t);
    for (size_t i = 0; i < g->n; i++) {
        clear_value(&values[i]);
    }
    return err;
}

static inline int handle_implicit_declare(struct group *equ, struct value *v)
{
    char **d, **dep = NULL;
    size_t ndep = 0;
    struct group *var, *impl, *c;

    var = &equ->g[0].g[0];
    if (var->t != GROUP_VARIABLE) {
        return 1;
    }

    impl = &equ->g[0].g[1];
    switch (impl->t) {
    case GROUP_VARIABLE:
        /* function depending on a single variable */
        v->v.var = add_variable(var->v.w, &equ->g[1], &impl->v.w, 1);
        break;

    case GROUP_ROUND:
        /* function depending on any amount of variables */
        c = impl->g;
        while (c->t == GROUP_COMMA) {
            if (c->g[1].t != GROUP_VARIABLE) {
                free(dep);
                return 1;
            }
            d = reallocarray(dep, ndep + 1, sizeof(*d));
            if (d == NULL) {
                free(dep);
                throw_error("%s", strerror(errno));
                return -1;
            }
            dep = d;
            dep[ndep++] = c->v.w;
            c = c->g;
        }
        if (c->t != GROUP_VARIABLE) {
            free(dep);
            return 1;
        }
        d = reallocarray(dep, ndep + 1, sizeof(*d));
        if (d == NULL) {
            throw_error("%s", strerror(errno));
            return -1;
        }
        dep = d;
        dep[ndep++] = c->v.w;
        v->v.var = add_variable(var->v.w, &equ->g[1], dep, ndep);
        free(dep);
        break;

    default:
        return 1;
    }
    return 0;
}

int compute_value(struct group *g, struct value *v)
{
    struct variable *var;

    switch (g->t) {
    case GROUP_EQUAL:
        v->t = VALUE_VARIABLE;
        switch (g->g[0].t) {
        case GROUP_VARIABLE:
            var = get_variable(g->g[0].v.w, 0);
            if (var == NULL) {
                v->v.var = add_variable(g->g[0].v.w, &g->g[1], NULL, 0);
            } else {
                clear_group(&var->value);
                copy_group(&var->value, &g->g[1]);
                v->v.var = var;
            }
            return 0;

        case GROUP_IMPLICIT:
            switch (handle_implicit_declare(g, v)) {
            case -1:
                return -1;
            case 0:
                return 0;
            case 1:
                break;
            }

        default:
            break;
        }
        break;

    default:
        break;
    }
    return compute_deep_value(g, v);
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
        if (v->v.mat.m == 1) {
            printf("( ");
            for (size_t i = 0; i < v->v.mat.n; i++) {
                output_value(&v->v.mat.v[i]);
                printf(" ");
            }
            printf(")");
        } else {
            for (size_t i = 0; i < v->v.mat.m; i++) {
                printf("\n");
                for (size_t j = 0; j < v->v.mat.n; j++) {
                    output_value(&v->v.mat.v[i * v->v.mat.n + j]);
                    printf(" ");
                }
            }
        }
        break;

    case VALUE_VARIABLE:
        printf("%s", v->v.var->name);
        if (v->v.var->ndep == 1) {
            printf(": %s", v->v.var->dep[0]);
        } else {
            printf("(%s", v->v.var->dep[0]);
            for (size_t i = 1; i < v->v.var->ndep; i++) {
                printf(", %s", v->v.var->dep[i]);
            }
            printf(")");
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
