#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

/*
 * Current token being analyzed by the parser
 */
extern token current_token;

/*
 * Number of syntax errors found by the parser
 */
extern int syntax_errors;


/*
 * Main parser entry point
 */
void system_goal(void);


/*
 * Structural grammar routines
 */
void program(void);
void statement_list(void);
void statement(void);
void id_list(void);
void expr_list(void);


/*
 * Expression routines
 *
 * These will be completed by Persona B
 */
void expression(void);
void primary(void);
void add_op(void);


/*
 * Parser utility functions
 */
void match(token expected);
void syntax_error(token actual);

#endif
