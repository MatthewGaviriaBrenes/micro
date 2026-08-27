/*
 * Micro Compiler
 * File: main.c
 * Responsibility: Todos
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "codegen.h"

int main(int argc, char *argv[])
{
    const char *input_filename;
    const char *output_filename;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.micro> <output.s>\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_filename = argv[1];
    output_filename = argv[2];

    if (freopen(input_filename, "r", stdin) == NULL) {
        fprintf(stderr, "Error: could not open input file '%s'\n",
                input_filename);
        return EXIT_FAILURE;
    }

    symtab_init();
    codegen_init(output_filename);

    current_token = scanner();
    system_goal();

    if (lexical_errors != 0 || syntax_errors != 0) {
        codegen_close();
        remove(output_filename);
        return EXIT_FAILURE;
    }

    codegen_end();
    codegen_close();

    return EXIT_SUCCESS;
}