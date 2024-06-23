#include "sc.h"
#include "dict.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool find_item(const char *word, size_t lenWord,
        const void *base, size_t size, size_t num,
        size_t *pIndex)
{
    size_t l, r;

    l = 0;
    r = num;
    while (l < r) {
        const size_t m = (l + r) / 2;
        char *const d = *(char**) (base + m * size);
        int cmp = strncmp(word, d, lenWord);
        if (cmp == 0 && d[lenWord] != '\0') {
            cmp = -1;
        }
        if (cmp == 0) {
            *pIndex = m;
            return true;
        }
        if (cmp < 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    *pIndex = r;
    return false;
}

static int add_item(void *item, size_t index,
        void **pBase, size_t size, size_t *pNum)
{
    void *newBase;

    newBase = realloc(*pBase, size * (*pNum + 1));
    if (newBase == NULL) {
        return -1;
    }
    *pBase = newBase;

    memmove(newBase + (index + 1) * size,
        newBase + index * size,
        size * (*pNum - index));
    memcpy(newBase + index * size, item, size);
    (*pNum)++;
    return 0;
}

Variable *all_variables;
size_t num_variables;

int put_variable(Variable *var)
{
    size_t index;

    if (find_item(var->name, strlen(var->name),
                all_variables, sizeof(*all_variables),
                num_variables, &index)) {
        delete_expression(all_variables[index].value);
        all_variables[index].value = var->value;
        return 1;
    }

    var->name = dict_put(var->name);
    if (var->name == NULL) {
        return -1;
    }

    if (add_item(var, index, (void**) &all_variables, sizeof(*all_variables),
                &num_variables) < 0) {
        return -1;
    }
    return 0;
}

int get_variable(const char *name, size_t lenName, Variable *var)
{
    size_t index;

    if (!find_item(name, lenName,
                all_variables, sizeof(*all_variables),
                num_variables, &index)) {
        return -1;
    }
    *var = all_variables[index];
    return 0;
}

FunctionSet *all_functions;
size_t num_functions;

int put_function(Function *func)
{
    FunctionSet newSet;
    size_t index;

    if (find_item(func->name, strlen(func->name),
                all_functions, sizeof(*all_functions),
                num_functions, &index)) {
        FunctionData *newData;

        FunctionSet *const set = &all_functions[index];
        for (size_t i = 0; i < set->numData; i++) {
            if (set->data[i].numArgs == func->data.numArgs) {
                delete_expression(set->data[i].returnValue);
                set->data[i].returnValue = func->data.returnValue;
            }
            return 1;
        }
        newData = realloc(set->data, sizeof(*set->data) *
                (set->numData + 1));
        if (newData == NULL) {
            return -1;
        }
        set->data = newData;
        set->data[set->numData++] = func->data;
        return 2;
    }

    func->name = dict_put(func->name);
    if (func->name == NULL) {
        return -1;
    }

    newSet.name = func->name;
    newSet.numData = 1;
    newSet.data = malloc(sizeof(*newSet.data));
    if (newSet.data == NULL) {
        return -1;
    }
    newSet.data[0] = func->data;
    if (add_item(&newSet, index,
                (void**) &all_functions, sizeof(*all_functions),
                &num_functions) < 0) {
        return -1;
    }
    return 0;
}

int get_function(const char *name, size_t lenName, FunctionSet *funcs)
{
    size_t index;

    if (!find_item(name, lenName, all_functions, sizeof(*all_functions),
                num_functions, &index)) {
        return -1;
    }
    *funcs = all_functions[index];
    return 0;
}
