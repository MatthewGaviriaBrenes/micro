#include <stdio.h>
#include <stdlib.h>

#include "../../src/parser.h"
#include "../../src/codegen.h"

int main(int argc, char *argv[])
{
    FILE *input;

    if (argc != 2) {
        fprintf(stderr,
                "Uso: %s <archivo.micro>\n",
                argv[0]);

        return EXIT_FAILURE;
    }

    input = freopen(argv[1], "r", stdin);

    if (input == NULL) {
        perror("No se pudo abrir el archivo");
        return EXIT_FAILURE;
    }

    codegen_init("expression_test.s");

    current_token = scanner();

    system_goal();

    codegen_end();
    codegen_close();

    fclose(input);

    if (lexical_errors > 0 || syntax_errors > 0) {

        fprintf(stderr,
                "\nCompilacion rechazada: "
                "%d error(es) lexico(s), "
                "%d error(es) sintactico(s).\n",
                lexical_errors,
                syntax_errors);

        return EXIT_FAILURE;
    }

    printf("Programa sintacticamente valido.\n");

    return EXIT_SUCCESS;
}