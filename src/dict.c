#include "dict.h"

#include <stdlib.h>
#include <string.h>

struct dictionary Dict;

static char *search_entryl(const char *word, size_t len, size_t *pIndex)
{
    size_t l, r;

    l = 0;
    r = Dict.num;
    while (l < r) {
        const size_t m = (l + r) / 2;

        int cmp = strncmp(Dict.words[m], word, len);
        if (cmp == 0 && word[len] != '\0') {
            cmp = -1;
        }
        if (cmp == 0) {
            if (pIndex != NULL) {
                *pIndex = m;
            }
            return Dict.words[m];
        }
        if (cmp < 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    if (pIndex != NULL) {
        *pIndex = r;
    }
    return NULL;
}

static char *search_entry(const char *word, size_t *pIndex)
{
    size_t l, r;

    l = 0;
    r = Dict.num;
    while (l < r) {
        const size_t m = (l + r) / 2;

        const int cmp = strcmp(Dict.words[m], word);
        if (cmp == 0) {
            if (pIndex != NULL) {
                *pIndex = m;
            }
            return Dict.words[m];
        }
        if (cmp < 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    if (pIndex != NULL) {
        *pIndex = r;
    }
    return NULL;
}

char *dict_putl(const char *word, size_t len)
{
    char *w;
    size_t index;

    w = search_entryl(word, len, &index);
    if (w != NULL) {
        return w;
    }

    char **const words =
        realloc(Dict.words, sizeof(*Dict.words) * (Dict.num + 1));
    if (words == NULL) {
        return NULL;
    }
    Dict.words = words;

    w = strndup(word, len);
    if (w == NULL) {
        return NULL;
    }
    memmove(&Dict.words[index + 1], &Dict.words[index],
            sizeof(*Dict.words) * (Dict.num - index));
    Dict.words[index] = w;
    Dict.num++;
    return w;
}

char *dict_put(const char *word)
{
    char *w;
    size_t index;

    w = search_entry(word, &index);
    if (w != NULL) {
        return w;
    }

    char **const words =
        realloc(Dict.words, sizeof(*Dict.words) * (Dict.num + 1));
    if (words == NULL) {
        return NULL;
    }
    Dict.words = words;

    w = strdup(word);
    if (w == NULL) {
        return NULL;
    }
    memmove(&Dict.words[index + 1], &Dict.words[index],
            sizeof(*Dict.words) * (Dict.num - index));
    Dict.words[index] = w;
    Dict.num++;
    return w;
}
