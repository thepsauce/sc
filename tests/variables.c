#include "test.h"
#include "dict.h"

int main(void)
{
    Variable v;

    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    v.name = dict_put("a");
    v.value = new_number("123");
    put_variable(&v);
    v.name = dict_put("b");
    v.value = new_number("13");
    put_variable(&v);

    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        if (line[lenLine - 1] == '\n') {
            line[--lenLine] = '\0';
        }
        if (line[0] == '\0') {
            continue;
        }
        if (get_variable(line, lenLine, &v) < 0) {
            printf("variable '%s' does not exist!\n", line);
        } else {
            printf("%s = ", v.name);
            output_expression(v.value, stdout);
            printf("\n");
        }
    }
    return 0;
}
