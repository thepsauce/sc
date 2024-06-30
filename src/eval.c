#include "core.h"

void operator_null(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_positive(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_negate(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_not(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_plus(struct value *v, struct value *values)
{
    struct value *v1, *v2;
    if (values[0].t > values[1].t) {
        v1 = &values[0];
        v2 = &values[1];
    } else {
        v1 = &values[1];
        v2 = &values[0];
    }
    switch (v1->t) {
    case VALUE_NUMBER:
        switch (v2->t) {
        case VALUE_NUMBER:
            v->t = VALUE_NUMBER;
            mpf_init(v->v.f);
            mpf_add(v->v.f, v1->v.f, v2->v.f);
            break;
        default:
            break;
        }
    default:
        break;
    }
}

void operator_minus(struct value *v, struct value *values)
{
    struct value *v1, *v2;
    v1 = &values[0];
    v2 = &values[1];
    switch (v1->t) {
    case VALUE_NUMBER:
        switch (v2->t) {
        case VALUE_NUMBER:
            v->t = VALUE_NUMBER;
            mpf_init(v->v.f);
            mpf_sub(v->v.f, v1->v.f, v2->v.f);
            break;
        default:
            break;
        }
    default:
        break;
    }
}

void operator_multiply(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_divide(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_mod(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_raise(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_lower(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_and(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_or(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_xor(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_if(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_less(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_less_than(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_greater(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_greater_than(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_equals(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_not_equals(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_comma(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_semicolon(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_do(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_where(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_exclam(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_percent(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_else(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_round(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_double_corner(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_corner(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_square(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_curly(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_double_bar(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_bar(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operator_choose_from(struct value *v, struct value *values)
{
    (void) v; (void) values;
}

void operate(struct value *v, struct value *values, enum group_type opr)
{
    void (*operators[])(struct value *v, struct value *values) = {
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

        [GROUP_ELEMENT_OF] = NULL,

        /* expr expr */
        [GROUP_IMPLICIT] = NULL,

        [GROUP_VARIABLE] = NULL,
        [GROUP_NUMBER] = NULL,
    };
    (*operators[opr])(v, values);
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
        operate(v, values, g->t);
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
    case GROUP_EQUALS:
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
