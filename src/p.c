#include "parse.h"
#include "macros.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <wchar.h>

struct parser Parser;

static void throw_error(const char *msg, ...)
{
    while (Parser.p != Parser.s) {
        Parser.p--;
        fprintf(stderr, " ");
    }
    fprintf(stderr, "~\n");

    va_list l;
    va_start(l, msg);
    vfprintf(stderr, msg, l);
    va_end(l);
    fprintf(stderr, "\n");

    longjmp(Parser.jb, 0);
}

static void skip_space(void)
{
    while (isspace(Parser.p[0])) {
        Parser.p++;
    }
}

static void read_word(void)
{
    if (!isalpha(Parser.p[0])) {
        throw_error("expected word");
    }
    Parser.w = Parser.p;
    while (isalnum(Parser.p[0])) {
        Parser.p++;
    }
    Parser.n = Parser.p - Parser.w;
}

static void read_number(void)
{
    if (gmp_sscanf(Parser.p, "%Ff%n", Parser.f, &Parser.n) != 1) {
        throw_error("expected number");
    }
    Parser.p += Parser.n;
}

static size_t begins_with(const char *s)
{
    size_t n = 0;
    for (; *s != '\0'; s++) {
        if (*s != Parser.p[n]) {
            return 0;
        }
        n++;
    }
    return n;
}

static void read_expression(int p /* precedence */)
{
    /* opr expr */
    static const struct prefix_operator {
        const char *s;
        int p;
    } prefixes[] = {
        { "+", 5 },
        { "-", 5 },
        { "negate", 5 },
        { "not", 4 },
    };

    /* expr opr expr */
    static const struct infix_operator {
        const char *s;
        int p;
    } infixes[] = {
        { "*", 6 },
        { "+", 5 },
        { ",", 1 },
        { "-", 5 },
        { "/", 6 },
        { ";", 2 },
        { "<", 3 },
        { "<=", 3 },
        { "=", 3 },
        { ">", 3 },
        { ">=", 3 },
        { "^", 7 },
        { "≠", 3 },
        { "≤", 3 },
        { "≥", 3 },
        { "and", 3 },
        { "do", 7 },
        { "equals", 3 },
        { "equals", 3 },
        { "greater", 3 },
        { "greater than", 3 },
        { "if", 4 },
        { "less", 3 },
        { "less than", 3 },
        { "mod", 6 },
        { "or", 3 },
        { "where", 4 },
        { "xor", 3 },
    };

    /* expr opr */
    static const struct suffix_operator {
        const char *s;
        int p;
    } suffixes[] = {
        { "!", 8 },
        { "%", 8 },
        { "else", 2 },
    };

    /* opr expr opr */
    static const struct match_operator {
        const char *l, *r;
    } matches[] = {
        { "(", ")" },
        { "<<", ">>" },
        { "<", ">" },
        { "[", "]" },
        { "{", "}" },
        { "||", "||" },
        { "|", "|" },
    };

    /* opr expr opr expr */
    static const struct unique_operator {
        const char *s1, *s2;
        int p;
    } uniques[] = {
        { "choose", "from", 4 },
    };

    Parser.e = NULL;

    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(prefixes); i++) {
        const size_t n = begins_with(prefixes[i].s);
        if (n > 0) {
            if (prefixes[i].p <= p) {
                return;
            }
            Parser.p += n;
            read_expression(prefixes[i].p);
            if (Parser.e == NULL) {
                throw_error("missing expression after operator");
            }
            skip_space();
            break;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(uniques); i++) {
        const size_t n = begins_with(uniques[i].s1);
        if (n > 0) {
            if (uniques[i].p <= p) {
                return;
            }
            Parser.p += n;
            read_expression(uniques[i].p);
            skip_space();
            const size_t n = begins_with(uniques[i].s2);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        uniques[i].s2, uniques[i].s1);
            }
            Parser.p += n;
            read_expression(uniques[i].p);
            goto got_value;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(matches); i++) {
        const size_t n = begins_with(matches[i].l);
        if (n > 0) {
            Parser.p += n;
            read_expression(0);
            skip_space();
            const size_t n = begins_with(matches[i].r);
            if (n == 0) {
                throw_error("missing matching '%s' for '%s'",
                        matches[i].r, matches[i].l);
            }
            Parser.p += n;
            goto got_value;
        }
    }

    if (isalpha(Parser.p[0])) {
        read_word();
        Parser.e = (void*) 1;
    } else if (isdigit(Parser.p[0]) || Parser.p[0] == '.') {
        read_number();
        Parser.e = (void*) 1;
    } else {
        throw_error("need value");
    }

got_value:
    skip_space();

    for (size_t i = 0; i < ARRAY_SIZE(infixes); i++) {
        const size_t n = begins_with(infixes[i].s);
        if (n > 0) {
            if (infixes[i].p <= p) {
                return;
            }
            Parser.p += n;
            read_expression(infixes[i].p);
            goto got_value;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(suffixes); i++) {
        const size_t n = begins_with(suffixes[i].s);
        if (n > 0) {
            if (suffixes[i].p <= p) {
                return;
            }
            Parser.p += n;
            goto got_value;
        }
    }
}

int parse(void)
{
    mpf_init(Parser.f);
    if (setjmp(Parser.jb) > 0) {
        return -1;
    }
    Parser.p = Parser.s;
    read_expression(0);
    return 0;
}
