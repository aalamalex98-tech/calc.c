
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
#include <limits.h>
#include <stdint.h>

/* Store either an integer result or a floating-point result. */
typedef struct {
    long long integer;
    double real;
    int is_float;
} Value;

/* Track the expression and the first parsing error. */
typedef struct {
    const char *text;
    size_t length;
    size_t pos;
    size_t error_pos;
    int failed;
} Parser;

/* Record only the first error. */
static void set_error(Parser *p, size_t pos)
{
    if (!p->failed) {
        p->failed = 1;
        p->error_pos = pos;
    }
}

static Value make_integer(long long n)
{
    Value v = { n, (double)n, 0 };
    return v;
}

static Value make_float(double n)
{
    Value v = { 0, n, 1 };
    return v;
}

/* Skip spaces and tabs while preserving character positions. */
static void skip_spaces(Parser *p)
{
    while (p->pos < p->length &&
           (p->text[p->pos] == ' ' ||
            p->text[p->pos] == '\t')) {
        p->pos++;
    }
}

/* Parse an integer and detect values outside the integer range. */
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

/* Parse a decimal such as 12.5 or 0.75. */
static Value parse_decimal(Parser *p)
{
    skip_spaces(p);

    size_t start = p->pos;
    int digits_before = 0;
    int digits_after = 0;

    while (p->pos < p->length &&
           isdigit((unsigned char)p->text[p->pos])) {
        digits_before++;
        p->pos++;
    }

    if (p->pos < p->length && p->text[p->pos] == '.') {
        p->pos++;

        while (p->pos < p->length &&
               isdigit((unsigned char)p->text[p->pos])) {
            digits_after++;
            p->pos++;
        }
    }

    if (digits_before == 0 || digits_after == 0) {
    set_error(p, start + digits_before + 1);
    return make_float(0.0);
}
    size_t count = p->pos - start;
    char *number = malloc(count + 1);

    if (number == NULL) {
        set_error(p, start + 1);
        return make_float(0.0);
    }

    memcpy(number, p->text + start, count);
    number[count] = '\0';

    errno = 0;
    char *end = NULL;
    double result = strtod(number, &end);

    if (errno == ERANGE || end == number ||
        *end != '\0' || !isfinite(result)) {
        set_error(p, start + 1);
        free(number);
        return make_float(0.0);
    }

    free(number);
    return make_float(result);
}

/* Forward declaration allows parentheses to contain expressions. */
static Value parse_expression(Parser *p);


/*
 * Parse a number or parenthesized expression.
 * Unary plus and minus are not allowed by the assignment.
 */
static Value parse_primary(Parser *p)
{
    skip_spaces(p);

    /* Reject unary signs at the beginning of an expression. */
    if (p->pos < p->length &&
        (p->text[p->pos] == '+' ||
         p->text[p->pos] == '-')) {
        set_error(p, p->pos + 1);
        return make_integer(0);
    }

    /* Parse a parenthesized expression. */
    if (p->pos < p->length &&
        p->text[p->pos] == '(') {

        p->pos++;

        Value result = parse_expression(p);
        skip_spaces(p);

        if (p->failed) {
            return make_integer(0);
        }

        if (p->pos >= p->length ||
            p->text[p->pos] != ')') {
            set_error(p, p->pos + 1);
            return make_integer(0);
        }

        p->pos++;
        return result;
    }

    /* A number must begin with a digit. */
    if (p->pos >= p->length ||
        !isdigit((unsigned char)p->text[p->pos])) {
        set_error(p, p->pos + 1);
        return make_integer(0);
    }

    size_t i = p->pos;

    while (i < p->length &&
           isdigit((unsigned char)p->text[i])) {
        i++;
    }

    Value result;

    if (i < p->length && p->text[i] == '.') {
        result = parse_decimal(p);
    } else {
        result = parse_integer(p);
    }

    if (p->failed) {
        return make_integer(0);
    }

    return result;
}



/* Parse exponentiation from right to left: 2**3**2 = 2**(3**2). */
static Value parse_power(Parser *p)
{
    Value base = parse_primary(p);
    skip_spaces(p);

    if (p->pos + 1 < p->length &&
        p->text[p->pos] == '*' &&
        p->text[p->pos + 1] == '*') {

        size_t operator_pos = p->pos;
        p->pos += 2;

        Value exponent = parse_power(p);

        if (p->failed) {
            return make_float(0.0);
        }

        if (exponent.real < 0.0) {
    set_error(p, operator_pos + 4);
    return make_float(0.0);
}

errno = 0;
double result = pow(base.real, exponent.real);

if (errno == ERANGE || !isfinite(result)) {
    set_error(p, operator_pos + 1);
    return make_float(0.0);
}

        return make_float(result);
    }

    return base;
}

/* Parse multiplication and division after exponentiation. */
static Value parse_term(Parser *p)
{
    Value result = parse_power(p);

    while (!p->failed) {
        skip_spaces(p);

        if (p->pos >= p->length) {
            break;
        }

        char op = p->text[p->pos];

        if (op != '*' && op != '/') {
            break;
        }

        /* Two stars together belong to exponentiation. */
        if (op == '*' &&
            p->pos + 1 < p->length &&
            p->text[p->pos + 1] == '*') {
            break;
        }

        p->pos++;
        skip_spaces(p);

        size_t right_start = p->pos;
        Value right = parse_power(p);

        if (p->failed) {
            break;
        }

        if (op == '/' && right.real == 0.0) {
            set_error(p, right_start + 1);
            break;
        }

        if (!result.is_float && !right.is_float) {
            if (op == '/') {
                if (result.integer == LLONG_MIN &&
                    right.integer == -1) {
                    set_error(p, right_start + 1);
                    break;
                }

                result = make_integer(
                    result.integer / right.integer
                );
            } else {
                long long a = result.integer;
                long long b = right.integer;

                if (a != 0 && b != 0 &&
                    ((a == -1 && b == LLONG_MIN) ||
                     (b == -1 && a == LLONG_MIN) ||
                     (a > 0 && b > 0 && a > LLONG_MAX / b) ||
                     (a > 0 && b < 0 && b < LLONG_MIN / a) ||
                     (a < 0 && b > 0 && a < LLONG_MIN / b) ||
                     (a < 0 && b < 0 && a < LLONG_MAX / b))) {
                    set_error(p, right_start + 1);
                    break;
                }

                result = make_integer(a * b);
            }
        } else {
            double value = (op == '*')
                ? result.real * right.real
                : result.real / right.real;

            if (!isfinite(value)) {
                set_error(p, right_start + 1);
                break;
            }

            result = make_float(value);
        }
    }

    return result;
}

/* Parse addition and subtraction after multiplication and division. */
static Value parse_sum(Parser *p)
{
    Value result = parse_term(p);

    while (!p->failed) {
        skip_spaces(p);

        if (p->pos >= p->length) {
            break;
        }

        char op = p->text[p->pos];

        if (op != '+' && op != '-') {
            break;
        }

        p->pos++;
        skip_spaces(p);

        size_t right_start = p->pos;
        Value right = parse_term(p);

        if (p->failed) {
            break;
        }

        if (!result.is_float && !right.is_float) {
            long long a = result.integer;
            long long b = right.integer;
            int overflow = 0;

            if (op == '+') {
                if ((b > 0 && a > LLONG_MAX - b) ||
                    (b < 0 && a < LLONG_MIN - b)) {
                    overflow = 1;
                }
            } else {
                if ((b < 0 && a > LLONG_MAX + b) ||
                    (b > 0 && a < LLONG_MIN + b)) {
                    overflow = 1;
                }
            }

            if (overflow) {
                set_error(p, right_start + 1);
                break;
            }

            result = make_integer(op == '+'
                ? a + b
                : a - b);
        } else {
            double value = (op == '+')
                ? result.real + right.real
                : result.real - right.real;

            if (!isfinite(value)) {
                set_error(p, right_start + 1);
                break;
            }

            result = make_float(value);
        }
    }

    return result;
}

/* Parse a complete arithmetic expression. */
static Value parse_expression(Parser *p)
{
    return parse_sum(p);
}

/*
 * Read one complete line, growing the buffer when necessary.
 * This avoids imposing a small fixed maximum expression length.
 */
static char *read_line(FILE *input, size_t *line_length)
{
    size_t capacity = 128;
    size_t length = 0;
    char *line = malloc(capacity);

    if (line == NULL) {
        return NULL;
    }

    int ch;

    while ((ch = fgetc(input)) != EOF) {
        if (ch == '\n') {
            break;
        }

        if (length + 1 >= capacity) {
            if (capacity > (size_t)-1 / 2) {
                free(line);
                return NULL;
            }

            size_t new_capacity = capacity * 2;
            char *new_line = realloc(line, new_capacity);

            if (new_line == NULL) {
                free(line);
                return NULL;
            }

            line = new_line;
            capacity = new_capacity;
        }

        line[length++] = (char)ch;
    }

    if (ch == EOF && length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';
    *line_length = length;
    return line;
}

/*
 * Process one input file. Blank lines and comment lines are ignored.
 * Error positions count all characters from the beginning of the file.
 */
static int process_file(const char *input_name, const char *output_name)
{
    FILE *input = fopen(input_name, "r");

    if (input == NULL) {
        fprintf(stderr, "Cannot open input file: %s\n", input_name);
        return 1;
    }

    FILE *output = fopen(output_name, "w");

    if (output == NULL) {
        fprintf(stderr, "Cannot open output file: %s\n", output_name);
        fclose(input);
        return 1;
    }

    size_t file_position = 0;
    int status = 0;

    for (;;) {
        size_t line_length = 0;
        char *line = read_line(input, &line_length);

        if (line == NULL) {
            if (ferror(input)) {
                fprintf(stderr, "Error reading input file.\n");
                status = 1;
            }
            break;
        }

        /*
         * Determine whether the line ended with a newline.
         * read_line consumes the newline itself.
         */
        int next_char = fgetc(input);
        int has_newline = (next_char != EOF);

        if (has_newline && ungetc(next_char, input) == EOF) {
            fprintf(stderr, "Error reading input file.\n");
            free(line);
            status = 1;
            break;
        }

        size_t first = 0;

        while (first < line_length &&
               (line[first] == ' ' || line[first] == '\t' ||
                line[first] == '\r')) {
            first++;
        }

        /* Ignore blank lines and comment lines. */
        if (first < line_length && line[first] != '#') {
            Parser parser = {
                line, line_length, 0, 0, 0
            };

            Value result = parse_expression(&parser);
            skip_spaces(&parser);

            if (!parser.failed && parser.pos < parser.length) {
                set_error(&parser, parser.pos + 1);
            }

            if (parser.failed) {
    size_t absolute_error =
        file_position + parser.error_pos;

    /*
     * Discard results written before the error.
     * The output must contain only the first error.
     */
    if (fclose(output) != 0) {
        fprintf(stderr, "Error writing output file.\n");
        free(line);
        status = 1;
        break;
    }

    output = fopen(output_name, "w");

    if (output == NULL) {
        fprintf(stderr, "Cannot reopen output file: %s\n",
                output_name);
        free(line);
        status = 1;
        break;
    }

    if (fprintf(output, "ERROR:%zu\n",
                absolute_error) < 0) {
        status = 1;
    } else {
        status = 2;
    }

    free(line);
    break;
}

            if (result.is_float) {
                if (fprintf(output, "%.15g\n", result.real) < 0) {
                    status = 1;
                }
            } else {
                if (fprintf(output, "%lld\n", result.integer) < 0) {
                    status = 1;
                }
            }

            if (status == 1) {
                free(line);
                break;
            }
        }

        file_position += line_length;

        if (has_newline) {
            file_position++;
        }

        free(line);

        if (!has_newline) {
            break;
        }
    }

    if (fclose(input) != 0) {
        fprintf(stderr, "Error closing input file.\n");
        status = 1;
    }

    if (fclose(output) != 0) {
        fprintf(stderr, "Error writing output file.\n");
        status = 1;
    }

    return status == 2 ? 0 : status;
}


/* Join a directory and filename into one allocated path. */
static char *join_path(const char *directory, const char *filename)
{
    size_t dlen = strlen(directory);
    size_t flen = strlen(filename);
    int separator = dlen > 0 && directory[dlen - 1] != '/';

    if (dlen > SIZE_MAX - flen - (size_t)separator - 1) {
        return NULL;
    }

    size_t length = dlen + flen + (size_t)separator + 1;
    char *path = malloc(length);

    if (path == NULL) {
        return NULL;
    }

    snprintf(path, length, "%s%s%s",
             directory, separator ? "/" : "", filename);
    return path;
}

static const char *base_name(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash == NULL ? path : slash + 1;
}

static char *input_base_name(const char *path)
{
    const char *base = base_name(path);
    size_t length = strlen(base);

    if (length >= 4 && strcmp(base + length - 4, ".txt") == 0) {
        length -= 4;
    }

    char *result = malloc(length + 1);
    if (result == NULL) {
        return NULL;
    }

    memcpy(result, base, length);
    result[length] = '\0';
    return result;
}

static int ensure_directory(const char *path)
{
    struct stat info;

    if (stat(path, &info) == 0) {
        if (S_ISDIR(info.st_mode)) {
            return 0;
        }
        fprintf(stderr, "Output path is not a directory: %s\n", path);
        return 1;
    }

    if (mkdir(path, 0777) != 0) {
        perror(path);
        return 1;
    }

    return 0;
}

static char *make_output_path(const char *outdir, const char *input)
{
    char *base = input_base_name(input);
    if (base == NULL) {
        return NULL;
    }

    int needed = snprintf(NULL, 0, "%s_Alex_Aalam_251ADB029.txt", base);
    if (needed < 0) {
        free(base);
        return NULL;
    }

    char *filename = malloc((size_t)needed + 1);
    if (filename == NULL) {
        free(base);
        return NULL;
    }

    snprintf(filename, (size_t)needed + 1,
             "%s_Alex_Aalam_251ADB029.txt", base);
    free(base);

    char *path = join_path(outdir, filename);
    free(filename);
    return path;
}

static int process_directory(const char *directory, const char *outdir)
{
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        perror(directory);
        return 1;
    }

    int status = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        size_t length = strlen(entry->d_name);

        if (length < 4 ||
            strcmp(entry->d_name + length - 4, ".txt") != 0) {
            continue;
        }

        char *input_path = join_path(directory, entry->d_name);
        if (input_path == NULL) {
            status = 1;
            break;
        }

        struct stat info;
        if (stat(input_path, &info) != 0 || !S_ISREG(info.st_mode)) {
            free(input_path);
            continue;
        }

        char *output_path = make_output_path(outdir, input_path);
        if (output_path == NULL) {
            free(input_path);
            status = 1;
            break;
        }

        if (process_file(input_path, output_path) != 0) {
            status = 1;
        }

        free(input_path);
        free(output_path);
    }

    if (closedir(dir) != 0) {
        perror("Error closing input directory");
        status = 1;
    }

    return status;
}

int main(int argc, char *argv[])
{
    const char *input_file = NULL;
    const char *input_dir = NULL;
    const char *output_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 ||
            strcmp(argv[i], "--dir") == 0) {
            if (i + 1 >= argc || input_dir != NULL || input_file != NULL) {
                fprintf(stderr, "Invalid directory option.\n");
                return 1;
            }
            input_dir = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 ||
                   strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 >= argc || output_dir != NULL) {
                fprintf(stderr, "Invalid output option.\n");
                return 1;
            }
            output_dir = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            if (input_file != NULL || input_dir != NULL) {
                fprintf(stderr, "Specify one input file or one directory.\n");
                return 1;
            }
            input_file = argv[i];
        }
    }

    if ((input_file == NULL) == (input_dir == NULL)) {
        fprintf(stderr,
                "Usage: %s [-d DIR] [-o OUTDIR] [input.txt]\n", argv[0]);
        return 1;
    }

    char *default_dir = NULL;

    if (output_dir == NULL) {
        char *base = input_base_name(
            input_file != NULL ? input_file : input_dir);

        if (base == NULL) {
            return 1;
        }

        int needed = snprintf(NULL, 0, "%s_Alex_Aalam_251ADB029", base);
        if (needed < 0) {
            free(base);
            return 1;
        }

        default_dir = malloc((size_t)needed + 1);
        if (default_dir == NULL) {
            free(base);
            return 1;
        }

        snprintf(default_dir, (size_t)needed + 1,
                 "%s_Alex_Aalam_251ADB029", base);
        free(base);
        output_dir = default_dir;
    }

    if (ensure_directory(output_dir) != 0) {
        free(default_dir);
        return 1;
    }

    int status;

    if (input_dir != NULL) {
        status = process_directory(input_dir, output_dir);
    } else {
        char *output_path = make_output_path(output_dir, input_file);

        if (output_path == NULL) {
            free(default_dir);
            return 1;
        }

        status = process_file(input_file, output_path);
        free(output_path);
    }

    free(default_dir);
    return status;
}
