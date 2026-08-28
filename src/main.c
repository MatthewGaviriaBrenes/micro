#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scanner.h"
#include "parser.h"
#include "symtab.h"
#include "codegen.h"

#define MAX_PATH_LEN 4096

/* Wrap a path in single quotes for safe shell usage */
static void shell_quote(const char *src, char *dest, size_t dest_size)
{
    size_t di = 0;
    dest[di++] = '\'';
    for (size_t i = 0; src[i] != '\0' && di < dest_size - 6; i++) {
        if (src[i] == '\'') {
            dest[di++] = '\'';
            dest[di++] = '\\';
            dest[di++] = '\'';
            dest[di++] = '\'';
        } else {
            dest[di++] = src[i];
        }
    }
    dest[di++] = '\'';
    dest[di] = '\0';
}

/* Build .s and executable paths from the .m input path */
static int derive_output_paths(const char *input_path,
                                char *asm_path, size_t asm_size,
                                char *exe_path, size_t exe_size)
{
    const char *ext = ".m";
    size_t ext_len = strlen(ext);
    size_t len = strlen(input_path);
    size_t base_len;

    if (len <= ext_len || strcmp(input_path + len - ext_len, ext) != 0) {
        fprintf(stderr, "Error: input file must have a .m extension\n");
        return -1;
    }

    base_len = len - ext_len;

    if (base_len + 3 > asm_size || base_len + 1 > exe_size) {
        fprintf(stderr, "Error: input path too long\n");
        return -1;
    }

    memcpy(asm_path, input_path, base_len);
    strcpy(asm_path + base_len, ".s");
    memcpy(exe_path, input_path, base_len);
    exe_path[base_len] = '\0';
    return 0;
}

int main(int argc, char *argv[])
{
    char asm_path[MAX_PATH_LEN];
    char exe_path[MAX_PATH_LEN];
    char quoted_asm[MAX_PATH_LEN + 8];
    char quoted_exe[MAX_PATH_LEN + 8];
    char command[MAX_PATH_LEN * 2 + 64];
    const char *input_filename;
    int saved_stdin;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.m>\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_filename = argv[1];

    if (derive_output_paths(input_filename, asm_path, sizeof(asm_path),
                            exe_path, sizeof(exe_path)) != 0) {
        return EXIT_FAILURE;
    }

    /* Save the real stdin before redirecting it to the source file */
    saved_stdin = dup(STDIN_FILENO);
    if (saved_stdin == -1) {
        fprintf(stderr, "Error: could not save stdin\n");
        return EXIT_FAILURE;
    }

    if (freopen(input_filename, "r", stdin) == NULL) {
        fprintf(stderr, "Error: could not open input file '%s'\n",
                input_filename);
        close(saved_stdin);
        return EXIT_FAILURE;
    }

    symtab_init();
    codegen_init(asm_path);

    current_token = scanner();
    system_goal();

    if (lexical_errors != 0 || syntax_errors != 0) {
        fprintf(stderr,
                "Compilacion rechazada: %d error(es) lexico(s), %d error(es) sintactico(s).\n",
                lexical_errors, syntax_errors);
        codegen_close();
        remove(asm_path);
        close(saved_stdin);
        return EXIT_FAILURE;
    }

    codegen_end();
    codegen_close();

    /* Restore the real stdin so the generated program can read from the terminal */
    fflush(stdin);
    if (dup2(saved_stdin, STDIN_FILENO) == -1) {
        fprintf(stderr, "Error: could not restore stdin\n");
        close(saved_stdin);
        return EXIT_FAILURE;
    }
    close(saved_stdin);

    /* Assemble and link the generated x86 code */
    shell_quote(asm_path, quoted_asm, sizeof(quoted_asm));
    shell_quote(exe_path, quoted_exe, sizeof(quoted_exe));

    snprintf(command, sizeof(command), "gcc %s -o %s", quoted_asm, quoted_exe);

    if (system(command) != 0) {
        fprintf(stderr, "Error: assembly/linking failed\n");
        remove(asm_path);
        remove(exe_path);
        return EXIT_FAILURE;
    }

    /* Run the executable, prefix with ./ if it has no directory component */
    if (strchr(exe_path, '/') == NULL) {
        snprintf(command, sizeof(command), "./%s", quoted_exe);
    } else {
        snprintf(command, sizeof(command), "%s", quoted_exe);
    }

    if (system(command) == -1) {
        fprintf(stderr, "Error: could not execute generated program\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}