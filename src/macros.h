#ifndef MACROS_H
#define MACROS_H

/*
 * Get the number of elements of a static array.
 */
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))

/*
 * Get the size of a literal string, usage:
 * size_t s1 = STRING_SIZE("String"),
 * s2 = STRING_SIZE("Bye");
 *
 * And used no other way!
 */
#define STRING_SIZE(s) (sizeof(s)-1)

#endif

