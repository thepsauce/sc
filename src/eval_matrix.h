int matrix_plus_matrix(struct value *v, struct value *values)
{
    struct matrix *m1, *m2, dest;
    m1 = &values[0].v.m;
    m2 = &values[1].v.m;
    if (m1->m != m2->m || m1->n != m2->n) {
        throw_error("adding matrices of incompatible size");
        return -1;
    }

    dest.m = m1->m;
    dest.n = m2->n;
    dest.v = reallocarray(NULL, dest.m * dest.n, sizeof(*dest.v));
    if (dest.v == NULL) {
        throw_error("%s", strerror(errno));
        return -1;
    }

    struct value vs[2];
    for (size_t i = 0; i < m1->m; i++) {
        for (size_t j = 0; j < m1->n; j++) {
            const size_t ind = j + i * m1->n;
            vs[0] = m1->v[ind];
            vs[1] = m2->v[ind];
            if (operate(&dest.v[ind], vs, 2, GROUP_PLUS) == -1) {
                return -1;
            }
        }
    }
    v->t = VALUE_MATRIX;
    v->v.m = dest;
    return 0;
}

int matrix_minus_matrix(struct value *v, struct value *values)
{
    struct matrix *m1, *m2, dest;
    m1 = &values[0].v.m;
    m2 = &values[1].v.m;
    if (m1->m != m2->m || m1->n != m2->n) {
        throw_error("subtracting matrices of incompatible size");
        return -1;
    }

    dest.m = m1->m;
    dest.n = m1->n;
    dest.v = reallocarray(NULL, dest.m * dest.n, sizeof(*dest.v));
    if (dest.v == NULL) {
        throw_error("%s", strerror(errno));
        return -1;
    }

    struct value vs[2];
    for (size_t i = 0; i < m1->m; i++) {
        for (size_t j = 0; j < m1->n; j++) {
            const size_t ind = j + i * m1->n;
            vs[0] = m1->v[ind];
            vs[1] = m2->v[ind];
            if (operate(&dest.v[ind], vs, 2, GROUP_MINUS) == -1) {
                return -1;
            }
        }
    }
    v->t = VALUE_MATRIX;
    v->v.m = dest;
    return 0;
}

int matrix_multiply_number(struct value *v, struct value *values)
{
    struct matrix dest;

    struct matrix *m = &values[0].v.m;

    const size_t n = m->m * m->n;

    dest.v = reallocarray(NULL, n, sizeof(*dest.v));
    if (dest.v == NULL) {
        throw_error("%s", strerror(errno));
        return -1;
    }

    struct value vs[2];
    vs[0] = values[1];
    for (size_t i = 0; i < n; i++) {
        vs[1] = m->v[i];
        if (operate(&dest.v[i], vs, 2, GROUP_MULTIPLY) == -1) {
            return -1;
        }
    }

    dest.m = m->m;
    dest.n = m->n;

    v->t = VALUE_MATRIX;
    v->v.m = dest;
    return 0;
}

int number_multiply_matrix(struct value *v, struct value *values)
{
    struct value vs[2];

    vs[0] = values[1];
    vs[1] = values[0];
    return matrix_multiply_number(v, vs);
}

int matrix_multiply_matrix(struct value *v, struct value *values)
{
    struct matrix *m1, *m2, dest;
    m1 = &values[0].v.m;
    m2 = &values[1].v.m;
    if (m1->m != m2->n) {
        throw_error("multiplying matrices of incompatible size");
        return -1;
    }

    dest.m = m1->m;
    dest.n = m2->n;
    dest.v = reallocarray(NULL, dest.m * dest.n, sizeof(*dest.v));
    if (dest.v == NULL) {
        throw_error("%s", strerror(errno));
        return -1;
    }

    struct value sum, prod;
    struct value vs[2];
    for (size_t i = 0; i < m1->m; i++) {
        for (size_t j = 0; j < m2->n; j++) {
            for (size_t k = 0; k < m1->n; k++) {
                vs[0] = m1->v[i * m1->n + k];
                vs[1] = m2->v[k * m2->n + j];
                if (operate(&prod, vs, 2, GROUP_MULTIPLY) == -1) {
                    return -1;
                }
                if (k == 0) {
                    sum = prod;
                } else {
                    vs[0] = sum;
                    vs[1] = prod;
                    if (operate(&sum, vs, 2, GROUP_PLUS) == -1) {
                        return -1;
                    }
                }
            }
            dest.v[i * dest.n + j] = sum;
        }
    }
    v->t = VALUE_MATRIX;
    v->v.m = dest;
    return 0;
}

int vector_dot_product(struct value *v, struct value *values)
{
    struct matrix *v1, *v2;
    size_t n1, n2;

    v1 = &values[0].v.m;
    v2 = &values[1].v.m;
    if ((v1->m != 1 && v1->n != 1) || (v2->m != 1 && v2->n != 1)) {
        throw_error("dot product only for matrices with a single row/column");
        return -1;
    }
    if (v1->m == 1) {
        n1 = v1->n;
    } else {
        n1 = v1->m;
    }
    if (v2->m == 1) {
        n2 = v2->n;
    } else {
        n2 = v2->m;
    }
    if (n1 != n2) {
        throw_error("incompatible matrices for dot product");
        return -1;
    }

    struct value sum, ss;
    struct value vs[2];
    for (size_t i = 0; i < n1; i++) {
        vs[0] = v1->v[i];
        vs[1] = v2->v[i];
        if (operate(&ss, vs, 2, GROUP_PLUS) == -1) {
            return -1;
        }
        if (i == 0) {
            sum = ss;
        } else {
            vs[0] = sum;
            vs[1] = ss;
            if (operate(&sum, vs, 2, GROUP_PLUS) == -1) {
                return -1;
            }
        }
    }
    *v = sum;
    return 0;
}
