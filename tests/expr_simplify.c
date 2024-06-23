#include "test.h"

int main(void)
{
    Expression *expr;
    Expression *simple;

    expr = new_expression(EXPR_ADD);
    expr->value.duo.left = new_expression(EXPR_ADD);
    expr->value.duo.left->value.duo.left = new_number("1");
    expr->value.duo.left->value.duo.right = new_expression(EXPR_FACTORIAL);
    expr->value.duo.left->value.duo.right->value.expr = new_number("5");
    expr->value.duo.right = new_number("4.13");
    output_expression(expr, stdout);
    printf("\n");

    simple = simplify_expression(expr);
    output_expression(simple, stdout);
    printf("\n");

    delete_expression(simple);
    delete_expression(expr);

    expr = new_expression(EXPR_AROUND_BAR);
    expr->value.expr = new_expression(EXPR_RANGE);
    expr->value.expr->value.range.start = new_number("2");
    expr->value.expr->value.range.end = new_number("21");
    output_expression(expr, stdout);
    printf("\n");

    simple = simplify_expression(expr);
    output_expression(simple, stdout);
    printf("\n");

    delete_expression(simple);
    delete_expression(expr);

    return 0;
}
