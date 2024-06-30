void number_negate(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_neg(v->v.f, values[0].v.f);
}

void number_plus_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_add(v->v.f, values[0].v.f, values[1].v.f);
}

void number_minus_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_sub(v->v.f, values[0].v.f, values[1].v.f);
}

void number_multiply_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_mul(v->v.f, values[0].v.f, values[1].v.f);
}

void number_divide_number(struct value *v, struct value *values)
{
    v->t = VALUE_NUMBER;
    mpf_init(v->v.f);
    mpf_div(v->v.f, values[0].v.f, values[1].v.f);
}
