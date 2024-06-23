#include "sc.h"

#include <stdlib.h>

static Expression *expr_negate_number(const Expression *opr)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    mpf_init(expr->value.number.value);
    mpf_neg(expr->value.number.value,
            opr->value.number.value);
    return expr;
}

static Expression *expr_factorial_number(const Expression *opr)
{
    unsigned long n;
    mpz_t fac;
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    n = mpf_get_ui(opr->value.number.value);
    mpz_init(fac);
    mpz_fac_ui(fac, n);
    mpf_init(expr->value.number.value);
    mpf_set_z(expr->value.number.value, fac);
    mpz_clear(fac);
    return expr;
}

static Expression *expr_abs_number(const Expression *opr)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL)
        return NULL;
    expr->type = EXPR_NUMBER;
    mpf_init(expr->value.number.value);
    mpf_abs(expr->value.number.value,
            opr->value.number.value);
    return expr;
}

static Expression *expr_det_matrix(const Expression *opr)
{/*
    */
    (void) opr;
    return NULL;
}

static Expression *expr_length_vector(const Expression *opr)
{/*
    Expression *root;
    Expression *expr;


    for (size_t i = 0; i < opr->vector.numComponents; i++) {
        expr = simplify_expression(opr->vector.components[i]);
    }
    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        mpf_clear(root);
        return NULL;
    }
    expr = simplify_expression(root);*/
    (void) opr;
    return NULL;
}

static Expression *expr_size_set(const Expression *opr)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    mpf_init_set_ui(expr->value.number.value,
            opr->value.vector.numComponents);
    return expr;
}

static Expression *expr_size_range(const Expression *opr)
{
    Expression wrap, tmp, one;
    Expression *expr;

    one.type = EXPR_NUMBER;
    mpf_init_set_ui(one.value.number.value, 1);

    tmp.type = EXPR_SUB;
    tmp.value.duo.left = opr->value.range.end;
    tmp.value.duo.right = opr->value.range.start;

    /* size = end - start + 1 */
    wrap.type = EXPR_ADD;
    wrap.value.duo.left = &tmp;
    wrap.value.duo.right = &one;
    output_expression(&wrap, stdout);
    printf("\n");
    expr = simplify_expression(&wrap);
    mpf_clear(one.value.number.value);
    return expr;
}

static Expression *compute1(const struct expression_meta *meta,
        const Expression *opr)
{
    Expression *(*expr_negators[EXPR_MAX])(const Expression *opr) = {
        [EXPR_NUMBER] = expr_negate_number,
    };
    Expression *(*expr_factorial[EXPR_MAX])(const Expression *opr) = {
        [EXPR_NUMBER] = expr_factorial_number,
    };
    Expression *(*expr_around_bar[EXPR_MAX])(const Expression *opr) = {
        [EXPR_NUMBER] = expr_abs_number,
        [EXPR_VECTOR] = expr_length_vector,
        [EXPR_MATRIX] = expr_det_matrix,
        [EXPR_SET] = expr_size_set,
        [EXPR_RANGE] = expr_size_range,
    };
    switch (meta->exprType) {
    case EXPR_FACTORIAL:
        return (*expr_factorial[opr->type])(opr);
    case EXPR_NEGATE:
        return (*expr_negators[opr->type])(opr);
    case EXPR_AROUND_BAR:
        return (*expr_around_bar[opr->type])(opr);
    default:
        return NULL;
    }
}

static Expression *expr_add_numbers(const Expression *opr1,
        const Expression *opr2)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    mpf_init(expr->value.number.value);
    mpf_add(expr->value.number.value,
            opr1->value.number.value,
            opr2->value.number.value);
    return expr;
}

static Expression *expr_sub_numbers(const Expression *opr1,
        const Expression *opr2)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    mpf_init(expr->value.number.value);
    mpf_sub(expr->value.number.value,
            opr1->value.number.value,
            opr2->value.number.value);
    return expr;
}

static Expression *
compute2(const struct expression_meta *meta,
        Expression *opr1, Expression *opr2)
{
    static Expression *(*expr_adders[EXPR_MAX][EXPR_MAX])(
            const Expression *opr1, const Expression *opr2) = {
        [EXPR_NUMBER][EXPR_NUMBER] = expr_add_numbers,
    };
    static Expression *(*expr_subbers[EXPR_MAX][EXPR_MAX])(
            const Expression *opr1, const Expression *opr2) = {
        [EXPR_NUMBER][EXPR_NUMBER] = expr_sub_numbers,
    };
    Expression *(*func)(const Expression *opr1, const Expression *opr2);

    switch (meta->exprType) {
    case EXPR_ADD:
        func = expr_adders[opr1->type][opr2->type];
        if (func == NULL) {
            return NULL;
        }
        return (*func)(opr1, opr2);
    case EXPR_SUB:
        func = expr_subbers[opr1->type][opr2->type];
        if (func == NULL) {
            return NULL;
        }
        return (*func)(opr1, opr2);
    default:
        return NULL;
    }
}

Expression *simplify_expression(Expression *expr)
{
    const struct expression_meta *meta;
    Expression *newExpr = NULL;

    meta = get_meta(expr->type);
    switch (meta->type) {
    case EXPR_META_NULL:
        break;
    case EXPR_META_S:
    case EXPR_META_R:
    case EXPR_META_L: {
        Expression *const opr = simplify_expression(expr->value.expr);
        if (opr == NULL) {
            break;
        }
        newExpr = compute1(meta, opr);
        if (newExpr == NULL) {
            newExpr = malloc(sizeof(*newExpr));
            if (newExpr == NULL) {
                delete_expression(opr);
                break;
            }
            newExpr->type = expr->type;
            newExpr->value.expr = opr;
        }
        break;
    }
    case EXPR_META_M: {
        Expression *const opr1 = simplify_expression(expr->value.duo.left);
        if (opr1 == NULL) {
            break;
        }
        Expression *const opr2 = simplify_expression(expr->value.duo.right);
        if (opr2 == NULL) {
            delete_expression(opr1);
            break;
        }
        newExpr = compute2(meta, opr1, opr2);
        if (newExpr == NULL) {
            newExpr = malloc(sizeof(*newExpr));
            if (newExpr == NULL) {
                delete_expression(opr1);
                delete_expression(opr2);
                break;
            }
            newExpr->type = expr->type;
            newExpr->value.duo.left = opr1;
            newExpr->value.duo.right = opr2;
        }
        break;
    }
    case EXPR_VARIABLE:
        break;
    case EXPR_NUMBER:
        newExpr = malloc(sizeof(*newExpr));
        if (newExpr == NULL) {
            return NULL;
        }
        newExpr->type = EXPR_NUMBER;
        newExpr->value.number = expr->value.number;
        break;
    case EXPR_VECTOR:
        break;
    case EXPR_MATRIX:
        break;
    case EXPR_RANGE: {
        Expression *const start = simplify_expression(expr->value.range.start);
        if (start == NULL) {
            break;
        }
        Expression *const end = simplify_expression(expr->value.range.end);
        if (end == NULL) {
            delete_expression(start);
            break;
        }
        newExpr = malloc(sizeof(*newExpr));
        if (newExpr == NULL) {
            delete_expression(start);
            delete_expression(end);
            break;
        }
        newExpr->type = expr->type;
        newExpr->value.range.start = start;
        newExpr->value.range.end = end;
        break;
    }
    case EXPR_SET:
        break;
    case EXPR_FUNCTION_CALL:
        break;
    }
    return newExpr;
}
