#ifndef DICT_H
#define DICT_H

#include <stddef.h>

/* * * Dictionary * * */
/*
 * Use this utility to get better memory management for strings and
 * easier comparisons, e.g. string comparisons for two strings s1, s2
 * become s1 == s2 instead of the usual strcmp(s1, s2) == 0.
 */

extern struct dictionary {
    char **words;
    size_t num;
} Dict;

/*
 * Put the word into the dictionary, a pointer to the
 * dictionary entry is returned, this pointer shall never be freed
 */
char *dict_put(const char *word);

/*
 * Same as dict_put but use an additional parameter for the length of the word,
 * so that it does not need to be null terminated (null terminators are still
 * considered).
 */
char *dict_putl(const char *word, size_t len);

#endif

