/*
 * Micro Compiler
 * File: parser.c
 * Responsibility: Felipe y Matthew
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "semantics.h"
#include "symtab.h"

token current_token;
int syntax_errors = 0;

/* Convert a token to its name */
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

/* Recover until the end of a statement */
static void synchronize_statement(void)
{
    while (current_token != SEMICOLON &&
            current_token != END &&
            current_token != SCANEOF) {

        current_token = scanner();
    }
}

/* Report an unexpected token */
void syntax_error(token actual)
{
    syntax_errors++;
    fprintf(stderr,
            "Error sintactico: token inesperado %s.\n",
            token_name(actual));
}


/* Report an unexpected token with the expected token */
void syntax_error_expected(token expected, token actual)
{
    syntax_errors++;
    fprintf(stderr,
            "Error sintactico: se esperaba %s, "
            "pero se encontro %s.\n",
            token_name(expected),
            token_name(actual));
}

/* Match the expected token */
void match(token expected)
{
    if (current_token == expected) {
        current_token = scanner();
        return;
    }
    syntax_errors++;
    fprintf(stderr,
            "Error sintactico: se esperaba %s, "
            "pero se encontro %s.\n",
            token_name(expected),
            token_name(current_token));
    if (current_token != SCANEOF) {
        current_token = scanner();
    }
}

/* Parse the complete input */
void system_goal(void)
{
    program();
    match(SCANEOF);
}

/* Parse a complete Micro program */
void program(void)
{
    if (current_token != BEGIN) {
        syntax_error_expected(BEGIN, current_token);

        while (current_token != ID &&
                current_token != READ &&
                current_token != WRITE &&
                current_token != END &&
                current_token != SCANEOF) {
            current_token = scanner();
        }
    }
    if (current_token == BEGIN) {
        match(BEGIN);
    }
    statement_list();
    match(END);
}

/* Parse a list of statements */
void statement_list(void)
{
    if (current_token != ID &&
        current_token != READ &&
        current_token != WRITE) {
        syntax_error(current_token);
        return;
    }
    statement();
    while (current_token == ID ||
            current_token == READ ||
            current_token == WRITE) {
        statement();
    }
}

/* Parse one statement */
void statement(void)
{
    switch (current_token) {
        case ID:
            match(ID);
            if (current_token != ASSIGNOP) {
                syntax_error_expected(ASSIGNOP, current_token);
                synchronize_statement();
                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }
                return;
            }
            match(ASSIGNOP);
            expression();

            if (current_token != SEMICOLON) {
                syntax_error_expected(SEMICOLON, current_token);
                return;
            }
            match(SEMICOLON);
            break;

        case READ:
            match(READ);
            if (current_token != LPAREN) {
                syntax_error(current_token);
                synchronize_statement();
                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }
                return;
            }
            match(LPAREN);
            id_list();
            match(RPAREN);

            if (current_token != SEMICOLON) {
                syntax_error_expected(SEMICOLON, current_token);
                return;
            }
            match(SEMICOLON);
            break;

        case WRITE:
            match(WRITE);
            if (current_token != LPAREN) {
                syntax_error(current_token);
                synchronize_statement();
                if (current_token == SEMICOLON) {
                    match(SEMICOLON);
                }
                return;
            }
            match(LPAREN);
            expr_list();
            match(RPAREN);

            if (current_token != SEMICOLON) {
                syntax_error_expected(SEMICOLON, current_token);
                return;
            }
            match(SEMICOLON);
            break;

        default:
            syntax_error(current_token);
            if (current_token != SCANEOF) {
                current_token = scanner();
            }
            break;
    }
}

/* Parse a list of identifiers */
void id_list(void)
{
    if (current_token == ID) {
        check_identifier(token_buffer);
        match(ID);
    } else {
        match(ID);
        return;
    }
    while (current_token == COMMA) {
        match(COMMA);
        if (current_token == ID) {
            check_identifier(token_buffer);
            match(ID);
        } else {
            match(ID);
        }
    }
}

/* Parse a list of expressions */
void expr_list(void)
{
    expression();
    while (current_token == COMMA) {
        match(COMMA);
        expression();
    }
}

/* Parse an expression */
SemanticValue expression(void)
{
    SemanticValue left;
    SemanticValue right;
    int operator;
    left = primary();
    while (current_token == PLUSOP ||
            current_token == MINUSOP) {
        operator = add_op();
        right = primary();
        left = gen_infix(left, operator, right);
    }
    return left;
}

/* Parse a primary expression */
SemanticValue primary(void)
{
    SemanticValue result;
    result.is_constant = 0;
    result.value = 0;
    
    if (current_token == INTLITERAL) {
        result = make_constant(atoi(token_buffer));
        match(INTLITERAL);
        return result;
    }

    if (current_token == ID) {
        check_identifier(token_buffer);
        match(ID);
        return result;
    }

    if (current_token == LPAREN) {
        match(LPAREN);
        result = expression();
        match(RPAREN);
        return result;
    }

    syntax_error(current_token);
    return result;
}


/* Parse an addition or subtraction operator */
int add_op(void)
{
    if (current_token == PLUSOP) {
        match(PLUSOP);
        return '+';
    }

    if (current_token == MINUSOP) {
        match(MINUSOP);
        return '-';
    }

    syntax_error(current_token);
    return 0;
}