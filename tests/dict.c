#include "test.h"
#include "dict.h"

#include <stdlib.h>
#include <string.h>

extern struct dictionary Dict;

void dict_dump(void)
{
    printf("%zu words:\n", Dict.num);
    for (size_t i = 0; i < Dict.num; i++) {
        printf("  %s\n", Dict.words[i]);
    }
}

int main(void)
{
    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        lenLine--;
        if (lenLine == 0) {
            continue;
        }
        line[lenLine] = '\0';
        if (strcmp(line, "dump") == 0) {
            dict_dump();
            continue;
        }
        char *const ent = dict_put(line);
        printf("%p, %s\n", ent, ent);
    }
    return 0;
}
