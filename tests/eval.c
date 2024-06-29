#include "parse.h"
#include "sc.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        line[lenLine - 1] = '\0';
        Parser.s = line;
        if (strcmp(line, "quit") == 0) {
            break;
        }
        struct group *g = new_group(1);
        if (parse(g) == 0) {
            /*while (Parser.p != Parser.s) {
                Parser.p--;
                printf(" ");
            }
            printf("~\n");*/
            //printf("syntax correct!\n");
            /*Expression *s = simplify_expression(Parser.e);
            output_expression(s, stdout);
            printf("\n");
            delete_expression(s);
            delete_expression(Parser.e);*/
            output_group(g, 0);

            printf(" = ");

            struct value v;
            compute_value(g, &v);
            output_value(&v);
            printf("\n");
            delete_value(&v);
            free_group(g);
        }
    }

    return 0;
}
