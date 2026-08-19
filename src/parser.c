/*
 * Micro Compiler
 * File: parser.c
 * Responsibility: Felipe y Matthew
 *
 */

#include <stdio.h>

#include "parser.h"


/*
 * Token that the parser is currently processing
 *
 * The scanner generates the tokens and the parser consumes
 * them one by one using this variable
 */
token current_token;


/*
 * Global counter for syntax errors found
 * during the analysis of the program
 */
int syntax_errors = 0;


/*
 * Converts a token to a string
 *
 * It is mainly used to generate understandable error messages for the user
 *
 */
static const char *token_name(token t)
{
    switch (t) {
        case BEGIN:       return "BEGIN";
        case END:         return "END";
        case READ:        return "READ";
        case WRITE:       return "WRITE";
        case ID:          return "ID";
        case INTLITERAL:  return "INTLITERAL";
        case LPAREN:      return "LPAREN";
        case RPAREN:      return "RPAREN";
        case SEMICOLON:   return "SEMICOLON";
        case COMMA:       return "COMMA";
        case ASSIGNOP:    return "ASSIGNOP";
        case PLUSOP:      return "PLUSOP";
        case MINUSOP:     return "MINUSOP";
        case SCANEOF:     return "SCANEOF";
        default:          return "UNKNOWN";
    }
}


/*
 * Recovery from Syntax Errors
 *
 * This prevents a large number of secondary errors
 * caused by a single initial error
 */
static void synchronize_statement(void)
{
    while (current_token != SEMICOLON &&
           current_token != END &&
           current_token != SCANEOF) {

        current_token = scanner();
    }
}


/*
 * Report a syntax error when the current token
 * cannot appear in the position where it is found
 */
void syntax_error(token actual)
{
    syntax_errors++;

    fprintf(stderr,
            "Error sintactico: token inesperado %s.\n",
            token_name(actual));
}


/*
 * Report a syntax error indicating explicitly
 * what token was expected and what was found
 *
 * This is more informative for the user and
 * facilitates locating errors in the Micro program
 */
void syntax_error_expected(token expected, token actual)
{
    syntax_errors++;

    fprintf(stderr,
            "Error sintactico: se esperaba %s, "
            "pero se encontro %s.\n",
            token_name(expected),
            token_name(actual));
}


/*
 * Checks that the current token is the expected token
 *
 * This function is used by practically all
 * the productions of the parser
 */
void match(token expected)
{
    if (current_token == expected) {

        /* The expected token was found */
        current_token = scanner();

        return;
    }


    /*
     * The found token does not match the expected one
     * Register the error and continue with the next token
     */
    syntax_errors++;

    fprintf(stderr,
            "Error sintactico: se esperaba %s, "
            "pero se encontro %s.\n",
            token_name(expected),
            token_name(current_token));


    /*
     * If we haven't reached the end of the file yet,
     * consume the unexpected token
     *
     * This prevents the parser from getting stuck
     */
    if (current_token != SCANEOF) {
        current_token = scanner();
    }
}


/*
 * Initial production of the parser
 *
 * First, a complete program is analyzed, and then it is verified 
 * that there are no additional tokens
 */
void system_goal(void)
{
    program();

    match(SCANEOF);
}


/*
 * Analyze the complete structure of a Micro program
 */
void program(void)
{
    /*
     * First check that the program starts with BEGIN
     */
    if (current_token != BEGIN) {

        syntax_error_expected(BEGIN, current_token);


        /*
         * Recovery when BEGIN is missing
         */
        while (current_token != ID &&
               current_token != READ &&
               current_token != WRITE &&
               current_token != END &&
               current_token != SCANEOF) {

            current_token = scanner();
        }
    }


    /*
     * If we find BEGIN, consume it
     */
    if (current_token == BEGIN) {
        match(BEGIN);
    }


    /*
     * A list of instructions must follow “BEGIN”
     */
    statement_list();


    /*
     * The program must end with "END"
     */
    match(END);
}


/*
 * Analyze a list of instructions
 *
 * While we find any of those tokens, continue processing instructions
 */
void statement_list(void)
{
    /*
     * The list must start with a statement
     */
    if (current_token != ID &&
        current_token != READ &&
        current_token != WRITE) {

        syntax_error(current_token);

        return;
    }


    /*
     * Process the first instruction
     */
    statement();


    /*
     * Process the remaining instructions
     */
    while (current_token == ID ||
           current_token == READ ||
           current_token == WRITE) {

        statement();
    }
}


/*
 * Analyze an individual instruction
 */
void statement(void)
{
    switch (current_token) {


        /*
         * ID := expression;
         */
        case ID:

            /* Consume the identifier */
            match(ID);


            /*
             * After an identifier is used, it must be followed by the assignment operator
             */
            if (current_token != ASSIGNOP) {

                syntax_error_expected(ASSIGNOP, current_token);


                /*
                 * If we don't find :=, abandon this instruction
                 */
                synchronize_statement();


                /*
                 * If we find ;, consume it to continue with the next instruction
                 */
                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }

                return;
            }


            /* Consumes := */
            match(ASSIGNOP);


            /*
             * Analyze the expression of the assignment
             */
            expression();


            /*
             * Every assignment must end with a ;
             */
            match(SEMICOLON);

            break;
        case READ:

            /* Consume the reserved word READ */
            match(READ);


            /*
             * After READ comes '('
             */
            if (current_token != LPAREN) {

                syntax_error(current_token);

                synchronize_statement();

                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }

                return;
            }


            /* Consume '(' */
            match(LPAREN);


            /*
             * Process the list of identifiers
             */
            id_list();


            /* Expect ')'. */
            match(RPAREN);


            /* Every READ statement ends with ';' */
            match(SEMICOLON);

            break;
        case WRITE:

            /* Consume the reserved word WRITE */
            match(WRITE);


            /*
             * After WRITE comes '('
             */
            if (current_token != LPAREN) {

                syntax_error(current_token);

                synchronize_statement();

                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }

                return;
            }


            /* Consume '(' */
            match(LPAREN);


            /*
             * Process the list of expressions
             */
            expr_list();


            /* Expect ')' */
            match(RPAREN);


            /* Every WRITE statement ends with ';' */
            match(SEMICOLON);

            break;


        /*
         * If statement() receives a unexpected token, record the error and proceed
         */
        default:

            syntax_error(current_token);


            /*
             * consume the invalid token to avoid
             * getting stuck in a loop
             */
            if (current_token != SCANEOF) {
                current_token = scanner();
            }

            break;
    }
}


/*
 * Analyze a list of identifiers
 */
void id_list(void)
{
    /* There must be at least one identifier */
    match(ID);


    /*
     * While we find a comma, expect another identifier
     */
    while (current_token == COMMA) {

        match(COMMA);

        match(ID);
    }
}


/*
 * Analyze a list of expressions
 */
void expr_list(void)
{
    /*
     * There must be at least one expression
     */
    expression();


    /*
     * If a comma appears, there must be another expression
     */
    while (current_token == COMMA) {

        match(COMMA);

        expression();
    }
}

