#include "core.h"

void noop(struct value *v, struct value *values)
{
    (void) v;
    (void) values;
    /* does nothing */
}

void number_negate(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_neg(v->v.f, values[0].v.f);
}

void bool_not(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = !values[0].v.b;
}

void number_plus_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_add(v->v.f, values[0].v.f, values[1].v.f);
}

void number_minus_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_sub(v->v.f, values[0].v.f, values[1].v.f);
}

void number_multiply_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_mul(v->v.f, values[0].v.f, values[1].v.f);
}

void number_divide_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_div(v->v.f, values[0].v.f, values[1].v.f);
}

void bool_and_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b & values[1].v.b;
}

void bool_or_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b | values[1].v.b;
}

void bool_xor_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b ^ values[1].v.b;
}

void number_less_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp < 0;
}

void number_less_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp <= 0;
}

void number_greater_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp > 0;
}

void number_greater_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp >= 0;
}

void number_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp == 0;
}

void number_not_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp != 0;
}

void operate(struct value *v, struct value *values, size_t n, enum group_type opr)
{
    static void (*single_operators[GROUP_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_POSITIVE][VALUE_NUMBER] = noop,
        [GROUP_POSITIVE][VALUE_VECTOR] = noop,
        [GROUP_POSITIVE][VALUE_MATRIX] = noop,

        [GROUP_NEGATE][VALUE_NUMBER] = number_negate,
        [GROUP_NOT][VALUE_NUMBER] = bool_not,
    };

    static void (*double_operators[GROUP_MAX][VALUE_MAX][VALUE_MAX])(struct value *v, struct value *values) = {
        [GROUP_PLUS][VALUE_NUMBER][VALUE_NUMBER] = number_plus_number,
        [GROUP_MINUS][VALUE_NUMBER][VALUE_NUMBER] = number_minus_number,
        [GROUP_MULTIPLY][VALUE_NUMBER][VALUE_NUMBER] = number_multiply_number,
        [GROUP_DIVIDE][VALUE_NUMBER][VALUE_NUMBER] = number_divide_number,

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
    case VALUE_VECTOR:
        break;
    case VALUE_MATRIX:
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
        compute_deeper_value(&var->value, v);
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
    case GROUP_IMPLICIT:
        break;
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
    case VALUE_VECTOR:
        break;
    case VALUE_MATRIX:
        break;
    case VALUE_SET:
        break;
    case VALUE_RANGE:
        break;
    default:
        break;
    }
}
