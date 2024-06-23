#include "test.h"

#include <stdlib.h>
#include <string.h>

Expression *new_expression(enum expression_type type)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = type;
    return expr;
}

Expression *new_number(const char *number)
{
    Expression *expr;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_NUMBER;
    mpf_init_set_str(expr->value.number.value, number, 0);
    return expr;
}

Expression *new_set(Expression **exprs, size_t numExprs)
{
    Set set;
    Expression *expr;

    set.members = malloc(sizeof(*set.members) * numExprs);
    if (set.members == NULL) {
        return NULL;
    }
    memcpy(set.members, exprs, sizeof(*exprs) * numExprs);
    set.numMembers = numExprs;

    expr = malloc(sizeof(*expr));
    if (expr == NULL) {
        free(set.members);
        return NULL;
    }
    expr->type = EXPR_SET;
    expr->value.set = set;
    return expr;
}
