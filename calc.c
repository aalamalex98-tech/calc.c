
/*
 * Name: Alex Aalam
 * Student ID: 251ADB029
 * Repository: https://github.com/aalamalex98-tech/calc.c
 * Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
 * Resources used: OpenAI ChatGPT for programming guidance and debugging.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Store either an integer result or a floating-point result. */
typedef struct {
    long long integer;
    double real;
    int is_float;
} Value;

/* Keep track of the expression and the first parsing error. */
typedef struct {
    const char *text;
    size_t length;
    size_t pos;
    size_t error_pos;
    int failed;
} Parser;

/* Record the first error position only. */
static void set_error(Parser *p, size_t pos)
{
    if (!p->failed) {
        p->failed = 1;
        p->error_pos = pos;
    }
}

/* Create an integer value. */
static Value make_integer(long long n)
{
    Value v = { n, (double)n, 0 };
    return v;
}

/* Create a floating-point value. */
static Value make_float(double n)
{
    Value v = { 0, n, 1 };
    return v;
}

int main(void)
{
    Value whole = make_integer(12);
    Value decimal = make_float(3.5);

    printf("Integer value: %lld\n", whole.integer);
    printf("Decimal value: %.1f\n", decimal.real);

    return 0;
}
