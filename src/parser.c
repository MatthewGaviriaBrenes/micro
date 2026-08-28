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
        case BAROP:       return "BAROP";
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
        {
            expr_rec target;
            expr_rec result;

            target = process_id(token_buffer);
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
            result = expression();
            assign(target, result);

            if (current_token != SEMICOLON) {
                syntax_error_expected(SEMICOLON, current_token);
                return;
            }

            match(SEMICOLON);
            break;
        }

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
    expr_rec variable;
    variable = process_id(token_buffer);
    match(ID);

    if (codegen_is_active()) {
        read_id(variable);
    }

    while (current_token == COMMA) {
        match(COMMA);

        variable = process_id(token_buffer);
        match(ID);

        if (codegen_is_active()) {
            read_id(variable);
        }
    }
}

/* Parse a list of expressions */
void expr_list(void)
{
    expr_rec result;
    result = expression();

    if (codegen_is_active()) {
        write_expr(result);
    }

    while (current_token == COMMA) {
        match(COMMA);
        result = expression();

        if (codegen_is_active()) {
            write_expr(result);
        }
    }
}

/* Parse an expression */
expr_rec expression(void)
{
    expr_rec left;
    expr_rec right;
    SemanticValue left_sem;
    SemanticValue right_sem;
    token op;

    left = primary();

    while (current_token == PLUSOP ||
            current_token == MINUSOP) {

        op = add_op();
        right = primary();

        /* Constant folding */
        if (left.kind == LITERALEXPR &&
            right.kind == LITERALEXPR) {

            left_sem = make_constant(left.val);
            right_sem = make_constant(right.val);

            left_sem = gen_infix(left_sem,
                                op == PLUSOP ? '+' : '-',
                                right_sem);

            if (left_sem.is_constant) {
                left = process_lit(left_sem.value);
            }

        } else {

            /* Generate runtime expression */
            left = generate_infix(left, op, right);
        }
    }

    return left;
}

/* Parse a primary expression */
expr_rec primary(void)
{
    expr_rec result;

    if (current_token == ID) {
        result = process_id(token_buffer);
        match(ID);
        return result;
    }

    if (current_token == INTLITERAL) {
        result = process_lit(atoi(token_buffer));
        match(INTLITERAL);
        return result;
    }

    if (current_token == LPAREN) {
        expr_rec condition;
        expr_rec true_value;
        expr_rec false_value;

        match(LPAREN);

        condition = expression();

        if (current_token == BAROP) {
            match(BAROP);
            true_value = expression();
            match(BAROP);
            false_value = expression();
            match(RPAREN);
            return generate_conditional(condition,
                                        true_value,
                                        false_value);
        }

        match(RPAREN);
        return condition;
    }

    syntax_error(current_token);

    return process_lit(0);
}

/* Parse an addition or subtraction operator */
token add_op(void)
{
    token operator;
    if (current_token == PLUSOP) {
        operator = PLUSOP;
        match(PLUSOP);
        return operator;
    }

    if (current_token == MINUSOP) {
        operator = MINUSOP;
        match(MINUSOP);
        return operator;
    }
    syntax_error(current_token);
    return PLUSOP;
}