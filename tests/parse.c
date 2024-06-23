#include "sc.h"

int main(void)
{
    Expression *expr;

    while (1) {
        expr = input_expression(stdin);
        if (expr == NULL) {
            printf("invalid expression\n");
            continue;
        }
        output_expression(expr, stdout);
        printf("\n");
        delete_expression(expr);
    }
    return 0;
}
