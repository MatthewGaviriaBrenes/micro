/*
 * Micro Compiler
 * File: scanner.c
 * Responsibility: Felipe B.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "scanner.h"

#define TOKEN_BUFFER_SIZE 256

char token_buffer[TOKEN_BUFFER_SIZE];

static int token_buffer_index = 0;
int lexical_errors = 0;


/*
 * Adds one character to token_buffer
 */
static void buffer_char(int c)
{
    if (token_buffer_index < TOKEN_BUFFER_SIZE - 1) {
        token_buffer[token_buffer_index++] = (char)c;
        token_buffer[token_buffer_index] = '\0';
    }
}


/*
 * Clears token_buffer
 */
static void clear_buffer(void)
{
    token_buffer_index = 0;
    token_buffer[0] = '\0';
}


/*
 * Checks whether token_buffer contains a reserved word
 */
static token check_reserved(void)
{
    if (strcmp(token_buffer, "begin") == 0)
        return BEGIN;

    if (strcmp(token_buffer, "end") == 0)
        return END;

    if (strcmp(token_buffer, "read") == 0)
        return READ;

    if (strcmp(token_buffer, "write") == 0)
        return WRITE;

    return ID;
}


/*
 * Reports a lexical error
 */
static void lexical_error(int c)
{
    lexical_errors++;

    if (c == EOF) {
        fprintf(stderr,
                "Error lexico: fin de archivo inesperado.\n");
    } else {
        fprintf(stderr,
                "Error lexico: caracter no valido '%c'.\n",
                c);
    }
}


/*
 * Returns the next token from the input
 */
token scanner(void)
{
    int in_char;
    int c;
    int identifier_length;
    int identifier_too_long;

    clear_buffer();

    while ((in_char = getchar()) != EOF) {

        /*
         * Ignore whitespace
         */
        if (isspace((unsigned char)in_char)) {
            continue;
        }

        /*
         * Identifier or reserved word
         *
         * Identifiers may contain letters, digits and underscores,
         * but must begin with a letter
         */
        if (isalpha((unsigned char)in_char)) {

            identifier_length = 1;
            identifier_too_long = 0;

            buffer_char(in_char);

            while (1) {
                c = getchar();

                if (isalnum((unsigned char)c) || c == '_') {

                    identifier_length++;

                    if (identifier_length <= MAX_IDENTIFIER_LENGTH) {
                        buffer_char(c);
                    } else {
                        identifier_too_long = 1;
                    }

                } else {
                    break;
                }
            }

            if (c != EOF) {
                ungetc(c, stdin);
            }

            if (identifier_too_long) {
                lexical_errors++;

                fprintf(stderr,
                        "Error lexico: identificador demasiado largo "
                        "(maximo %d caracteres).\n",
                        MAX_IDENTIFIER_LENGTH);

                continue;
            }

            return check_reserved();
        }

        /*
         * Integer literal
         */
        if (isdigit((unsigned char)in_char)) {

            buffer_char(in_char);

            while (1) {
                c = getchar();

                if (isdigit((unsigned char)c)) {
                    buffer_char(c);
                } else {
                    break;
                }
            }

            if (c != EOF) {
                ungetc(c, stdin);
            }

            return INTLITERAL;
        }

        /*
         * Left parenthesis
         */
        if (in_char == '(') {
            return LPAREN;
        }

        /*
         * Right parenthesis
         */
        if (in_char == ')') {
            return RPAREN;
        }

        /*
         * Semicolon
         */
        if (in_char == ';') {
            return SEMICOLON;
        }

        /*
         * Comma
         */
        if (in_char == ',') {
            return COMMA;
        }

        /*
         * Addition operator
         */
        if (in_char == '+') {
            return PLUSOP;
        }

        /*
         * Assignment operator :=
         */
        if (in_char == ':') {

            c = getchar();

            if (c == '=') {
                return ASSIGNOP;
            }

            if (c != EOF) {
                ungetc(c, stdin);
            }

            lexical_error(in_char);
            continue;
        }

        /*
         * Minus operator or comment
         *
         * Comments begin with -- and continue to the end of the line
         */
        if (in_char == '-') {

            c = getchar();

            if (c == '-') {

                while ((c = getchar()) != '\n' && c != EOF) {
                    /* Skip comment */
                }

                continue;

            } else {

                if (c != EOF) {
                    ungetc(c, stdin);
                }

                return MINUSOP;
            }
        }

        /*
         * Invalid character
         */
        lexical_error(in_char);
    }

    return SCANEOF;
}
