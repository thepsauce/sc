#include "parse.h"
#include "sc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    init_parser();

    printf(">>> ");
    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        line[lenLine - 1] = '\0';
        if (strcmp(line, "quit") == 0) {
            break;
        }
        switch (parse(line)) {
        case PARSER_ERROR:
            printf("error\n>>> ");
            reset_parser();
            break;
        case PARSER_OK:
            output_group(Parser.st[0], 0);
            printf("\n");
            reset_parser();
            printf(">>> ");
            break;
        case PARSER_CONTINUE:
            if (Parser.st[0]->t == GROUP_NULL) {
                printf(">>> ");
                continue;
            }
            printf("... ");
            break;
        }
    }
    return 0;
}

