#include "macros.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

size_t generate_random_operator(char *buf, int *st)
{
    static const char *l[] = {
        "+",
        "-",
        "negate",
        "not",
        "√",
        "∛",
    };
    static const char *m[] = {
        ";",
        ",",
        "where",
        "if",
        "do",
        "and",
        "or",
        "xor",
        "equals",
        "greater equals",
        "greater than",
        "less equals",
        "less than",
        "<=",
        "<",
        "=",
        ">=",
        ">",
        "≠",
        "≤",
        "≥",
        "+",
        "-",
        "*",
        "·",
        "/",
        "mod",
        "∈",
        "^",
        "_",
    };
    static const char *r[] = {
        "!",
        "%",
        "²",
        "³",
        "else"
    };

    const char **g, *o;
    size_t n;

    switch (rand() * 3 / RAND_MAX) {
    case 0:
        g = l;
        n = ARRAY_SIZE(l);
        *st = 0;
        break;
    case 1:
        g = m;
        n = ARRAY_SIZE(m);
        *st = 1;
        break;
    case 2:
        g = r;
        n = ARRAY_SIZE(r);
        *st = 2;
        break;
    }
    o = g[rand() * n / RAND_MAX];
    strcpy(buf, o);
    return strlen(buf);
}

size_t generate_random_digits(char *buf)
{
    const size_t n = 1 + rand() * 32 / RAND_MAX;
    for (size_t i = 0; i < n; i++) {
        buf[i] = rand() * 10 / RAND_MAX + '0';
    }
    return n;
}

static inline void extend_buffer(char **b, size_t *n, size_t *c, char *a, size_t na)
{
    if (*n + na + 1 > *c) {
        *c *= 2;
        *c += na + 1;
        *b = realloc(*b, *c);
        if (*b == NULL) {
            exit(1);
        }
    }
    memcpy(*b + *n, a, na);
    *n += na;
}

int main(void)
{
    const unsigned t = 10000;
    char *buf;
    size_t cnt, cap;

    char b[128];
    size_t n;
    int st;

    const time_t seed = time(NULL);
    printf("seed: %ld\n", seed);
    srand(seed);

    cap = 1024;
    buf = malloc(cap);
    cnt = 0;

    init_parser();

    struct timespec start, end;

    n = generate_random_digits(b);
    extend_buffer(&buf, &cnt, &cap, b, n);
    for (unsigned i = 0; i < t; i++) {
        buf[cnt] = '\0';
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (parse(buf) != PARSER_OK) {
            printf("could not parse: %s\n", buf);
            exit(1);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        end.tv_sec -= start.tv_sec;
        end.tv_nsec -= start.tv_nsec;
        if (end.tv_nsec < 0) {
            end.tv_sec--;
            end.tv_nsec += 1000000000;
        }
        printf("%ld,", end.tv_nsec);
        reset_parser();
        n = generate_random_operator(b, &st);
        extend_buffer(&buf, &cnt, &cap, b, n);
        switch (st) {
        case 2:
            continue;
        }
        n = generate_random_digits(b);
        extend_buffer(&buf, &cnt, &cap, b, n);
    }
    printf("\n");
}
