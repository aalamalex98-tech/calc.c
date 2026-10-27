# calc.c

A C17 arithmetic expression parser with file input/output and automated tests.

## Requirements

- GCC
- C17 support
- Standard C library
- Math library

## Compilation

```bash
gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
```

## Usage

### Evaluate an input file

```bash
./calc input.txt
```

### Specify output directory

```bash
./calc -o output input.txt
./calc --output-dir output input.txt
```

### Process a directory

```bash
./calc -d input_directory -o output_directory
./calc --dir input_directory --output-dir output_directory
```

## Expression Features

- Integer and decimal literals
- Addition and subtraction
- Multiplication and division
- Exponentiation
- Parenthesized expressions
- Comments and blank lines
- Error reporting using ERROR:<position>

## Testing

Run parser tests:

```bash
sh calc-tests/run_tests.sh ./calc
```

Run CLI tests:

```bash
sh calc-tests/cli.sh ./calc
```

## Sanitizer Testing

Compile:

```bash
gcc -O1 -g -Wall -Wextra -std=c17 -fsanitize=address,undefined -o calc_san calc.c -lm
```

Run parser tests:

```bash
sh calc-tests/run_tests.sh ./calc_san
```

Run CLI tests:

```bash
sh calc-tests/cli.sh ./calc_san
```