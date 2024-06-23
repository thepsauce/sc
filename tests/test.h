#include "sc.h"

Expression *new_expression(enum expression_type type);
Expression *new_number(const char *number);
Expression *new_set(Expression **exprs, size_t numExprs);
