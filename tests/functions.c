#include "test.h"
#include "dict.h"

#include <stdlib.h>

int main(void)
{
    Function f;
    FunctionSet s;

    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    f.name = dict_put("f");
    f.data.args = malloc(sizeof(*f.data.args));
    f.data.args[0] = dict_put("x");
    f.data.numArgs = 1;
    f.data.returnValue = new_expression(EXPR_VARIABLE);
    f.data.returnValue->value.name = dict_put("x");
    put_function(&f);
    f.name = dict_put("g");
    f.data.args = malloc(sizeof(*f.data.args));
    f.data.args[0] = dict_put("x");
    f.data.numArgs = 1;
    f.data.returnValue = new_expression(EXPR_MUL);
    f.data.returnValue->value.duo.left = new_number("32");
    f.data.returnValue->value.duo.right = new_expression(EXPR_VARIABLE);
    f.data.returnValue->value.duo.right->value.name = dict_put("x");
    put_function(&f);

    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        ssize_t curLen;
        char *p;

        curLen = lenLine - 1;
        p = line;
        while (lenLine > 1) {
            lenLine--;
            if (get_function(p, lenLine, &s) >= 0) {
                for (size_t i = 0; i < s.numData; i++) {
                    FunctionData *const d = &s.data[i];
                    printf("%s(", s.name);
                    for (size_t a = 0; a < d->numArgs; a++) {
                        if (a > 0) {
                            printf(", ");
                        }
                        printf("%s", d->args[a]);
                    }
                    printf(") = ");
                    output_expression(d->returnValue, stdout);
                    printf("\n");
                }
                if (lenLine == curLen) {
                    break;
                }
                p += lenLine;
                curLen -= lenLine;
                lenLine = curLen + 1;
            }
        }
    }
    return 0;
}
