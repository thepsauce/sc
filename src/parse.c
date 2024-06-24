#include "sc.h"

#include <ctype.h>
#include <stdlib.h>

struct parser {
    int prev, cur;
    FILE *fp;
};

static int parser_getc(struct parser *parser)
{
    parser->prev = parser->cur;
    parser->cur = fgetc(parser->fp);
    return parser->cur;
}

static Expression *input_expression_recursive(struct parser *parser, unsigned precedence)
{
    Expression *root = NULL;
    int c;
    char buf[1024];
    size_t lenBuf;
    Expression *expr;
    const struct expression_meta *meta;

    c = parser->cur;
    while (c != EOF && c != '\n') {
        if (isspace(c)) {
            c = parser_getc(parser);
            continue;
        }

        if (isdigit(c)) {
            if (root != NULL) {
                break;
            }
            lenBuf = 0;
            do {
                buf[lenBuf++] = c;
            } while (c = parser_getc(parser), isdigit(c));
            buf[lenBuf++] = '\0';
            expr = malloc(sizeof(*expr));
            if (expr == NULL) {
                goto err;
            }
            expr->type = EXPR_NUMBER;
            mpf_init_set_str(expr->value.number.value, buf, 10);
            root = expr;
        } else {
            char opr[2];

            if (root == NULL) {
                break;
            }
            opr[0] = c;
            opr[1] = 0;
            meta = get_meta_sequence(opr);
            if (meta == NULL) {
                return NULL;
            }
            if (meta->precedence <= precedence) {
                return root;
            }
            expr = malloc(sizeof(*expr));
            if (expr == NULL) {
                goto err;
            }
            expr->type = meta->exprType;
            expr->value.duo.left = root;
            do {
                parser_getc(parser);
                expr->value.duo.right = input_expression_recursive(parser, meta->precedence);
                if (expr->value.duo.right == NULL && parser->cur != '\n') {
                    free(expr);
                    goto err;
                }
            } while (expr->value.duo.right == NULL);
            root = expr;
            c = parser->cur;
        }
    }
    return root;

err:
    delete_expression(root);
    return NULL;
}

Expression *input_expression(FILE *fp)
{
    struct parser parser;

    parser.fp = fp;
    parser_getc(&parser);
    return input_expression_recursive(&parser, 0);
}
