
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


/* Skip spaces and tabs before reading a number. */
static void skip_spaces(Parser *p)
{
    while (p->pos < p->length &&
           (p->text[p->pos] == ' ' || p->text[p->pos] == '\t')) {
        p->pos++;
    }
}

/* Parse an integer literal and detect values outside its range. */
static Value parse_integer(Parser *p)
{
    skip_spaces(p);

    if (p->pos >= p->length ||
        !isdigit((unsigned char)p->text[p->pos])) {
        set_error(p, p->pos + 1);
        return make_integer(0);
    }

    size_t start = p->pos;

    while (p->pos < p->length &&
           isdigit((unsigned char)p->text[p->pos])) {
        p->pos++;
    }

    size_t count = p->pos - start;
    char *number = malloc(count + 1);

    if (number == NULL) {
        set_error(p, start + 1);
        return make_integer(0);
    }

    memcpy(number, p->text + start, count);
    number[count] = '\0';

    errno = 0;
    char *end = NULL;
    long long result = strtoll(number, &end, 10);

    if (errno == ERANGE || end == number || *end != '\0') {
        set_error(p, start + 1);
        free(number);
        return make_integer(0);
    }

    free(number);
    return make_integer(result);
}

int main(void)
{
    const char *input = "12345";
    Parser parser = { input, strlen(input), 0, 0, 0 };

    Value result = parse_integer(&parser);

    if (parser.failed) {
        printf("ERROR:%zu\n", parser.error_pos);
        return 1;
    }

    printf("%lld\n", result.integer);
    return 0;
}

