#include <stdio.h>
#include <stdlib.h>

#include "../../src/scanner.h"


static const char *token_name(token t)
{
    switch (t) {
        case BEGIN:
            return "BEGIN";

        case END:
            return "END";

        case READ:
            return "READ";

        case WRITE:
            return "WRITE";

        case ID:
            return "ID";

        case INTLITERAL:
            return "INTLITERAL";

        case LPAREN:
            return "LPAREN";

        case RPAREN:
            return "RPAREN";

        case SEMICOLON:
            return "SEMICOLON";

        case COMMA:
            return "COMMA";

        case ASSIGNOP:
            return "ASSIGNOP";

        case PLUSOP:
            return "PLUSOP";

        case MINUSOP:
            return "MINUSOP";

        case SCANEOF:
            return "SCANEOF";

        default:
            return "UNKNOWN";
    }
}


int main(int argc, char *argv[])
{
    FILE *input;
    token current_token;

    if (argc != 2) {
        fprintf(stderr,
                "Uso: %s <archivo.micro>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "r");

    if (input == NULL) {
        perror("No se pudo abrir el archivo");
        return EXIT_FAILURE;
    }

    /*
     * scanner() reads from stdin, so redirect stdin
     * to the input Micro file.
     */
    if (freopen(argv[1], "r", stdin) == NULL) {
        perror("No se pudo redirigir la entrada");
        fclose(input);
        return EXIT_FAILURE;
    }

    fclose(input);

    do {
        current_token = scanner();

        printf("%-12s", token_name(current_token));

        if (current_token == ID ||
            current_token == INTLITERAL) {

            printf("  \"%s\"", token_buffer);
        }

        printf("\n");

    } while (current_token != SCANEOF);

    if (lexical_errors > 0) {
        fprintf(stderr,
                "\nSe encontraron %d error(es) lexico(s).\n",
                lexical_errors);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
