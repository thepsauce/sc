#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

int main(void)
{
    char *line;
    size_t n;
    struct {
        char *p;
        size_t n;
        size_t c;
    } entire = { NULL, 0, 0 };
    struct value v;
    const char *prompt = ">>> ";

    init_parser();

    while ((line = readline(prompt)) != NULL) {
        n = strlen(line);
        if (n == 0) {
            continue;
        }
        if (strcmp(line, "quit") == 0) {
            break;
        }

        if (entire.n + n + 1 > entire.c) {
            entire.c *= 2;
            entire.c += n + 1;
            entire.p = realloc(entire.p, entire.c);
            if (entire.p == NULL) {
                return 1;
            }
        }
        memcpy(&entire.p[entire.n], line, n);
        entire.n += n;
        entire.p[entire.n++] = '\n';

        switch (parse(line)) {
        case PARSER_ERROR:
            printf("error\n");
            reset_parser();
            entire.n = 0;
            prompt = ">>> ";
            break;
        case PARSER_OK:
            if (compute_value(Core.st[0], &v) == 0) {
                output_group(Core.st[0], 0);
                printf(" = ");
                output_value(&v);
                clear_value(&v);
                printf("\n");
            }
            reset_parser();
            entire.p[entire.n - 1] = '\0';
            entire.n = 0;
            add_history(entire.p);
            prompt = ">>> ";
            break;
        case PARSER_CONTINUE:
            prompt = "... ";
            break;
        }
        free(line);
    }
    free(entire.p);
    return 0;
}
