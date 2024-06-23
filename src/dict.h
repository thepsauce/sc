#ifndef DICT_H
#define DICT_H

#include <stddef.h>

extern struct dictionary {
    char **words;
    size_t num;
} Dict;

char *dict_put(const char *word);

#endif

