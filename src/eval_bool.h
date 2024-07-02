int bool_not(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = !values[0].v.b;
    return 0;
}

int bool_and_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b & values[1].v.b;
    return 0;
}

int bool_or_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b | values[1].v.b;
    return 0;
}

int bool_xor_bool(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    v->v.b = values[0].v.b ^ values[1].v.b;
    return 0;
}

int number_less_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp < 0;
    return 0;
}

int number_less_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp <= 0;
    return 0;
}

int number_greater_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp > 0;
    return 0;
}

int number_greater_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp >= 0;
    return 0;
}

int number_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp == 0;
    return 0;
}

int number_not_equal_number(struct value *v, struct value *values)
{
    v->t = VALUE_BOOL;
    const int cmp = mpf_cmp(values[0].v.f, values[1].v.f);
    v->v.b = cmp != 0;
    return 0;
}
