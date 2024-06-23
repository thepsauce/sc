#include "test.h"
#include "macros.h"

int main(void)
{
    Expression *root;
    Expression *expr;
    Expression *expressions[10];

    root = new_expression(EXPR_FACTORIAL);
    expr = new_expression(EXPR_MOD);
    expr->value.duo.left = new_number("3.4");
    expr->value.duo.right = new_number("1.2");
    root->value.expr = expr;
    expr = new_expression(EXPR_MUL);
    expr->value.duo.left = root;
    expr->value.duo.right = new_number("4.2");
    root = expr;
    output_expression(root, stdout);
    expr = simplify_expression(root);
    output_expression(expr, stdout);
    delete_expression(expr);
    delete_expression(root);

    for (size_t i = 0; i < ARRAY_SIZE(expressions); i++) {
        const char str[] = { i + '0', '\0' };
        expressions[i] = new_number(str);
    }
    expr = new_set(expressions, ARRAY_SIZE(expressions));
    output_expression(expr, stdout);
    delete_expression(expr);

    return 0;
}
