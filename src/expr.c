#include "sc.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

struct expression_meta meta_info[] = {
    [EXPR_NULL] = { 0, EXPR_NULL, 0, NULL },

    [EXPR_ADD] = { EXPR_META_M, EXPR_ADD, 5, "+" },
    [EXPR_SUB] = { EXPR_META_M, EXPR_SUB, 5, "-" },
    [EXPR_MUL] = { EXPR_META_M, EXPR_MUL, 10, "*" },
    [EXPR_DIV] = { EXPR_META_M, EXPR_DIV, 10, "/" },
    [EXPR_POW] = { EXPR_META_M, EXPR_POW, 15, "^" },
    [EXPR_EQUAL] = { EXPR_META_M, EXPR_EQUAL, 4, "=" },
    [EXPR_ELEMENT_OF] = { EXPR_META_M, EXPR_ELEMENT_OF, 3, "∈" },
    [EXPR_AND] = { EXPR_META_M, EXPR_AND, 2, "and" },
    [EXPR_MOD] = { EXPR_META_M, EXPR_MOD, 8, "mod" },
    [EXPR_WHERE] = { EXPR_META_M, EXPR_WHERE, 1, "where" },
    [EXPR_CHOOSE] = { EXPR_META_M, EXPR_CHOOSE, 9, "choose" },
    [EXPR_EXT] = { EXPR_META_M, EXPR_EXT, 20, "." },
    [EXPR_BAR] = { EXPR_META_M, EXPR_BAR, 7, "|" },

    [EXPR_AROUND_BAR] = { EXPR_META_S, EXPR_AROUND_BAR, 0, "| |" },
    [EXPR_AROUND_DOUBLE_BAR] = { EXPR_META_S, EXPR_AROUND_DOUBLE_BAR, 0, "|| ||" },

    [EXPR_NEGATE] = { EXPR_META_L, EXPR_NEGATE, 5, "-" },
    [EXPR_FACTORIAL] = { EXPR_META_R, EXPR_FACTORIAL, 16, "!" },

    [EXPR_VARIABLE] = { EXPR_META_VARIABLE, EXPR_VARIABLE, 0, NULL },
    [EXPR_NUMBER] = { EXPR_META_NUMBER, EXPR_NUMBER, 0, NULL },
    [EXPR_VECTOR] = { EXPR_META_VECTOR, EXPR_VECTOR, 0, NULL },
    [EXPR_SET] = { EXPR_META_SET, EXPR_SET, 0, NULL },
    [EXPR_MATRIX] = { EXPR_META_MATRIX, EXPR_MATRIX, 0, NULL },
    [EXPR_RANGE] = { EXPR_META_RANGE, EXPR_RANGE, 0, NULL },
    [EXPR_FUNCTION_CALL] = { EXPR_META_FUNCTION_CALL, EXPR_FUNCTION_CALL, 0, NULL },
};

const struct expression_meta *get_meta(enum expression_type type)
{
    return &meta_info[type];
}

const struct expression_meta *get_meta_sequence(const char *seq)
{
    const char *cur, *end;

    for (enum expression_type type = EXPR_NULL; type != EXPR_MAX; type++) {
        end = meta_info[type].sequence;
        if (end == NULL) {
            continue;
        }
        end--;
        do {
            cur = end + 1;
            end = strchr(cur, ' ');
            if (end == NULL) {
                end = cur + strlen(cur);
            }
            if (cur != NULL && strncmp(seq, cur, end - cur) == 0 &&
                    seq[end - cur] == '\0') {
                return &meta_info[type];
            }
        } while (*end != '\0');
    }
    return NULL;
}

static void output_set(const Set *set, FILE *fp)
{
    fprintf(fp, "{");
    for (size_t i = 0; i < set->numMembers; i++) {
        if (i > 0) {
            fprintf(fp, ",");
        }
        fprintf(fp, " ");
        output_expression(set->members[i], fp);
    }
    fprintf(fp, " }");
}

static void output_range(const Range *range, FILE *fp)
{
    fprintf(fp, "[");
    output_expression(range->start, fp);
    fprintf(fp, ", ");
    output_expression(range->end, fp);
    fprintf(fp, "]");
}

static void output_vec(const Vector *vec, FILE *fp)
{
    fprintf(fp, "(");
    for (size_t i = 0; i < vec->numComponents; i++) {
        if (i > 0) {
            fprintf(fp, ",");
        }
        fprintf(fp, " ");
        output_expression(vec->components[i], fp);
    }
    fprintf(fp, " )");
}

static void output_mat(const Matrix *mat, FILE *fp)
{
    fprintf(fp, "(");
    for (size_t r = 0; r < mat->numRows; r++) {
        if (r > 0) {
            fprintf(fp, ";");
        }
        fprintf(fp, " ");
        for (size_t c = 0; c < mat->numCols; c++) {
            if (c > 0) {
                fprintf(fp, ",");
            }
            fprintf(fp, " ");
            output_expression(mat->components[r * mat->numCols + c], fp);
        }
    }
    fprintf(fp, " )");
}

static int output_expression_recursive(const Expression *expr, FILE *fp, unsigned precedence)
{
    const struct expression_meta *meta;

    meta = get_meta(expr->type);
    switch (meta->type) {
    case EXPR_META_NULL:
        break;
    case EXPR_META_L:
        fprintf(fp, "%s", meta->sequence);
        output_expression_recursive(expr->value.expr, fp, meta->precedence);
        break;
    case EXPR_META_R:
        output_expression_recursive(expr->value.expr, fp, meta->precedence);
        fprintf(fp, "%s", meta->sequence);
        break;
    case EXPR_META_S: {
        const char *const sp = strchr(meta->sequence, ' ');
        fprintf(fp, "%.*s", (int) (sp - meta->sequence),
                meta->sequence);
        output_expression_recursive(expr->value.expr, fp, 0);
        fprintf(fp, "%s", sp + 1);
        break;
    }
    case EXPR_META_M:
        if (meta->precedence < precedence) {
            fprintf(fp, "(");
        }
        output_expression_recursive(expr->value.duo.left, fp, meta->precedence);
        fprintf(fp, " %s ", meta->sequence);
        output_expression_recursive(expr->value.duo.right, fp, meta->precedence);
        if (meta->precedence < precedence) {
            fprintf(fp, ")");
        }
        break;

    case EXPR_VARIABLE:
        fprintf(fp, "%s", expr->value.name);
        break;
    case EXPR_NUMBER:
        mpf_out_str(fp, 10, 0, expr->value.number.value);
        break;
    case EXPR_VECTOR:
        output_vec(&expr->value.vector, fp);
        break;
    case EXPR_MATRIX:
        output_mat(&expr->value.matrix, fp);
        break;
    case EXPR_RANGE:
        output_range(&expr->value.range, fp);
        break;
    case EXPR_SET:
        output_set(&expr->value.set, fp);
        break;
    case EXPR_FUNCTION_CALL:
        fprintf(fp, "%s(", expr->value.call.name);
        for (size_t i = 0; i < expr->value.call.numArgs; i++) {
            if (i > 0) {
                fprintf(fp, ", ");
            }
            output_expression_recursive(expr->value.call.args[i], fp, 0);
        }
        fprintf(fp, ")");
        break;
    }
    return 0;
}

int output_expression(const Expression *expr, FILE *fp)
{
    return output_expression_recursive(expr, fp, 0);
}

void delete_expression(Expression *expr)
{
    const struct expression_meta *meta;

    if (expr == NULL) {
        return;
    }
    meta = get_meta(expr->type);
    switch (meta->type) {
    case EXPR_META_NULL:
        break;
    case EXPR_META_M:
        delete_expression(expr->value.duo.left);
        delete_expression(expr->value.duo.right);
        break;
    case EXPR_META_L:
    case EXPR_META_R:
    case EXPR_META_S:
        delete_expression(expr->value.expr);
        break;
    case EXPR_VARIABLE:
        break;
    case EXPR_NUMBER:
        mpf_clear(expr->value.number.value);
        break;
    case EXPR_VECTOR: {
        const Vector vec = expr->value.vector;
        for (size_t i = 0; i < vec.numComponents; i++) {
            delete_expression(vec.components[i]);
        }
        break;
    }
    case EXPR_MATRIX: {
        const Matrix mat = expr->value.matrix;
        for (size_t r = 0; r < mat.numRows; r++) {
            for (size_t c = 0; c < mat.numCols; c++) {
                delete_expression(mat.components[r * mat.numCols + c]);
            }
        }
        break;
    }
    case EXPR_SET: {
        const Set set = expr->value.set;
        for (size_t i = 0; i < set.numMembers; i++) {
            delete_expression(set.members[i]);
        }
        break;
    }
    case EXPR_RANGE: {
        const Range range = expr->value.range;
        delete_expression(range.start);
        delete_expression(range.end);
        break;
    }
    case EXPR_FUNCTION_CALL:
        for (size_t i = 0; i < expr->value.call.numArgs; i++) {
            delete_expression(expr->value.call.args[i]);
        }
        break;
    }
    free(expr);
}
