#include "core.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

struct core Core;

void throw_error(const char *fmt, ...)
{
    va_list l;
    va_start(l, fmt);
    vfprintf(stderr, fmt, l);
    va_end(l);
    fputc('\n', stderr);
    longjmp(Core.jb, 0);
}

struct variable *get_variable(const struct group *g)
{
    for (size_t i = Core.nv; i > 0; ) {
        struct variable *const v = &Core.v[--i];
        if (g->t == v->name.t) {
            switch (g->t) {
            case GROUP_VARIABLE:
                if (g->v.w == v->name.v.w) {
                    return v;
                }
                break;
            case GROUP_LOWER:
            case GROUP_IMPLICIT:
            default:
                throw_error("not implemented: lower and implicit variables");
                break;
            }
        }
    }
    return NULL;
}

void add_variable(const struct group *name, const struct group *val)
{
    struct variable *v;

    v = reallocarray(Core.v, Core.nv + 1, sizeof(*Core.v));
    if (v == NULL) {
        goto err;
    }
    Core.v = v;
    v = &Core.v[Core.nv];
    if (copy_group(&v->name, name) == -1) {
        goto err;
    }
    if (copy_group(&v->value, val) == -1) {
        clear_group(&v->name);
        goto err;
    }
    Core.nv++;
    return;

err:
    throw_error("could not allocate variable: %s", strerror(errno));
}
