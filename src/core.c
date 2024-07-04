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
}

struct variable *get_variable(const char *name, size_t ndep)
{
    for (size_t i = Core.nv; i > 0; ) {
        struct variable *const v = Core.v[--i];
        if (v->ndep == ndep && name == v->name) {
            return v;
        }
    }
    return NULL;
}

struct variable *add_variable(char *name, const struct group *val,
        char *const *dep, size_t ndep)
{
    struct variable **v, *var;

    v = reallocarray(Core.v, Core.nv + 1, sizeof(*Core.v));
    if (v == NULL) {
        goto err;
    }
    Core.v = v;
    var = malloc(sizeof(*var));
    if (var == NULL) {
        goto err;
    }
    Core.v[Core.nv] = var;
    var->dep = reallocarray(NULL, ndep, sizeof(*var->dep));
    if (var->dep == NULL) {
        goto err;
    }
    if (copy_group(&var->value, val) == -1) {
        free(var->dep);
        free(var);
        goto err;
    }
    var->name = name;
    for (size_t i = 0; i < ndep; i++) {
        var->dep[i] = dep[i];
    }
    var->ndep = ndep;
    Core.nv++;
    return var;

err:
    throw_error("%s", strerror(errno));
    return NULL;
}
