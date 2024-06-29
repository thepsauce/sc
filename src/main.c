#include <stdlib.h>
#include <stdio.h>

/**
 * 1
 *
 * 2 * 4
 * 4 + 3
 * 2 ^ (8 + 3 * 2)
 * 3!
 * e*pi
 *
 * 3^5 mod 4^2
 *
 * a = 3
 * b = 2
 * a * b
 *
 * f(x) = x^2
 * f(33)
 *
 * ceil f0.1
 *
 * f(x) = {
 * 3 * x if x < 4,
 * 4 + 2 * x if x >= 4 a
 * }
 *
 * A = { 2, 3, 4, 5 }
 * f(A)
 * f([1, 10])
 *
 * L = {
 * I1 + I2 + I3 = 0,
 * U1 - I2 * R2 = 0,
 * I2 * R2 - I3 * R3 = 0
 * }
 *
 * L.U1 = 4
 * L.R2 = 3
 * L.R3 = 9
 *
 * L.I1
 *
 * L
 *
 * L2 = { Ax = 0 }
 * L2.x where A = (4, 7, 1)
 *
 * ( 1, 2, 3 ) * 2
 * ( 1, 2, 3 ; 2, 4, 5 ) * ( 4 ; 5 ; 7 )
 *
 * { x + y = 3 } where y ∈ [1, 10]
 *
 * M = { x * x + y * y = 1 }
 * M.x where y = 4
 * M.y where x = 2
 * M.x where y ∈ [1, 10]
 *
 * (p,q)=(3,4)
 * u = (2, 4)
 * (p,q)*(u+q)
 * ||(p,q)||+u*(p+q,p-q)
 *
 * cos(φ)=(u*v)/(||u||*||v||) where u=(u1,u2),v=(v1,v2)
 * cos(23°) where (u1,u2)=2,3 and v=(1,2)
 * cos(21°) where (u1,u2,v1,v2) ∈ [1,4]^4
 *
 * choose 3 from 10
 *
 * choose 2 from {1, 2, 3}
 * choose 4 from {99,88,33,22,101,102,103}
 * choose 3 from |{1, 9, 33, 12, 43, 0}|
 */

/*
 * Number
 *  1, -2, 4.2, 1e3, 4e-1, 2.1e2, .4, .2e3
 * Operator
 *  -x
 *  |x|
 *  ||x||
 *  x+y
 *  x-y
 *  x*y
 *  x/y
 *  x^y
 *  x!
 *  x | y
 *  x ∈ y
 *  x and y
 *  x mod y
 *  x where y
 *  choose x from y
 *  x.y
 * Variable
 *  A, B, Ma, b, c, a1, a2, a3
 *  π, e, τ
 * Variable Declaration
 *  a=3, b=4a, x/y=x+y, A={1,2,3}, B=[1,10]
 *  (a,b)=(2,3)
 * Function
 *  f, g, a2, p3
 * Function Declaration
 *  f(x)=xx, g2(y)=123y^3-9y
 * Conditional Function Declaration
 *  f(x)={x if x < 0, -x if x >= 0}
 *  f2(x)={x/2 if x|2, x if not x|2}
 * Function Call
 *  f(x), a2 3, f 4
 *  sin(22), cos(x), f({1,2,3})
 * Set/Range
 *  {1,2,3}, [3,9]
 * Variable Set/Range
 *  {x*y=2}, {x*x-y*y=4}, [n,2*n]
 * Set Member
 *  x ∈ [1,2,3]
 * Vector
 *  (1,2,3)
 * Matrix
 *  (1 2;9 8;12 8)
 * Expression
 *  1*3, 4+9, choose 2 from {1,2,3}, (1,2,3) * 4, [1,4]
 *  I={x=2,y=2x+4}, I.x, I.y, I, A={x+y=z}, A where x=1 and y=2
 *  K={Ax=o} where o={0}^columns(A), K where A=(1 2;-4 5)
 */

int main(void)
{
    char *line = NULL;
    size_t szLine = 0;
    ssize_t lenLine;

    while ((lenLine = getline(&line, &szLine, stdin)) > 0) {
        lenLine--;
        if (lenLine == 0) {
            continue;
        }
        line[lenLine] = '\0';
        printf("%s\n", line);
    }
    return 0;
}
