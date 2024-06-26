#include "parse.h"

void vv_add(struct value *v1, struct value *v2)
{
    struct value *d;
    if (v1->t > v2->t) {
        struct value *tmp = v1;
        v1 = v2;
        v1 = tmp;
        d = v2;
    } else {
        d = v1;
    }
    switch (v1->t) {
    case VALUE_NUMBER:
        switch (v2->t) {
        case VALUE_NUMBER:
            d->t = VALUE_NUMBER;
            mpf_add(d->v.f, v1->v.f, v2->v.f);
            break;
        default:
            break;
        }
    default:
        break;
    }
}

void vv_sub(struct value *v1, struct value *v2)
{
    struct value *d;
    if (v1->t > v2->t) {
        struct value *tmp = v1;
        v1 = v2;
        v1 = tmp;
        d = v2;
    } else {
        d = v1;
    }
    switch (v1->t) {
    case VALUE_NUMBER:
        switch (v2->t) {
        case VALUE_NUMBER:
            d->t = VALUE_NUMBER;
            mpf_sub(d->v.f, v1->v.f, v2->v.f);
            break;
        default:
            break;
        }
    default:
        break;
    }
}

void operator_null(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_positive(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_negate(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_not(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_plus(struct value *v, struct value *values, size_t n)
{
    *v = values[0];
    for (size_t i = 1; i < n; i++) {
        vv_add(v, &values[i]);
    }
}

void operator_minus(struct value *v, struct value *values, size_t n)
{
    *v = values[0];
    for (size_t i = 1; i < n; i++) {
        vv_sub(v, &values[i]);
    }
}

void operator_multiply(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_divide(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_mod(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_raise(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_lower(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_and(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_or(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_xor(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_if(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_less(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_less_than(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_greater(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_greater_than(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_equals(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_not_equals(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_comma(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_semicolon(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_do(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_where(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_exclam(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_percent(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_else(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_round(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_double_corner(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_corner(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_square(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_curly(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_double_bar(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_bar(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_choose_from(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operator_implicit(struct value *v, struct value *values, size_t n)
{
    (void) v; (void) values; (void) n;
}

void operate(struct value *v, struct value *values, size_t n, enum group_type opr)
{
    void (*operators[])(struct value *v, struct value *values, size_t n) = {
        [GROUP_NULL] = operator_null,

        /* opr expr */
        [GROUP_POSITIVE] = operator_positive,
        [GROUP_NEGATE] = operator_negate,
        [GROUP_NOT] = operator_not,

        /* expr opr expr */
        [GROUP_PLUS] = operator_plus,
        [GROUP_MINUS] = operator_minus,
        [GROUP_MULTIPLY] = operator_multiply,
        [GROUP_DIVIDE] = operator_divide,
        [GROUP_MOD] = operator_mod,
        [GROUP_RAISE] = operator_raise,
        [GROUP_LOWER] = operator_lower,

        [GROUP_AND] = operator_and,
        [GROUP_OR] = operator_or,
        [GROUP_XOR] = operator_xor,

        [GROUP_IF] = operator_if,

        [GROUP_LESS] = operator_less,
        [GROUP_LESS_THAN] = operator_less_than,
        [GROUP_GREATER] = operator_greater,
        [GROUP_GREATER_THAN] = operator_greater_than,
        [GROUP_EQUALS] = operator_equals,
        [GROUP_NOT_EQUALS] = operator_not_equals,

        [GROUP_COMMA] = operator_comma,
        [GROUP_SEMICOLON] = operator_semicolon,

        [GROUP_DO] = operator_do,
        [GROUP_WHERE] = operator_where,

        /* expr opr */
        [GROUP_EXCLAM] = operator_exclam,
        [GROUP_PERCENT] = operator_percent,
        [GROUP_ELSE] = operator_else,

        /* opr expr opr */
        [GROUP_ROUND] = operator_round,
        [GROUP_DOUBLE_CORNER] = operator_double_corner,
        [GROUP_CORNER] = operator_corner,
        [GROUP_SQUARE] = operator_square,
        [GROUP_CURLY] = operator_curly,
        [GROUP_DOUBLE_BAR] = operator_double_bar,
        [GROUP_BAR] = operator_bar,

        /* opr expr opr expr */
        [GROUP_CHOOSE_FROM] = operator_choose_from,

        /* expr expr */
        [GROUP_IMPLICIT] = operator_implicit,

        [GROUP_VARIABLE] = NULL,
        [GROUP_NUMBER] = NULL,
    };
    (*operators[opr])(v, values, n);
}

void delete_value(struct value *v)
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
    }
}

void compute_value(const struct group *g, struct value *v)
{
    struct value values[g->n];
    for (size_t i = 0; i < g->n; i++) {
        compute_value(&g->g[i], &values[i]);
    }
    switch (g->t) {
    case GROUP_NUMBER:
        v->t = VALUE_NUMBER;
        mpf_init_set(v->v.f, g->v.f);
        break;
    default:
        operate(v, values, g->n, g->t);
    }
    /* ignore first element */
    for (size_t i = 1; i < g->n; i++) {
        delete_value(&values[i]);
    }
}

void output_value(struct value *v)
{
    switch (v->t) {
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
    }
}
