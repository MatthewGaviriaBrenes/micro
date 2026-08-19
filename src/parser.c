/*
 * Micro Compiler
 * File: parser.c
 * Responsibility: Felipe y Matthew
 *
 */

#include <stdio.h>

#include "parser.h"


token current_token;
int syntax_errors = 0;


static const char *token_name(token t)
{
    switch (t) {
        case BEGIN:      return "BEGIN";
        case END:        return "END";
        case READ:       return "READ";
        case WRITE:      return "WRITE";
        case ID:         return "ID";
        case INTLITERAL: return "INTLITERAL";
        case LPAREN:     return "LPAREN";
        case RPAREN:     return "RPAREN";
        case SEMICOLON:  return "SEMICOLON";
        case COMMA:      return "COMMA";
        case ASSIGNOP:   return "ASSIGNOP";
        case PLUSOP:     return "PLUSOP";
        case MINUSOP:    return "MINUSOP";
        case SCANEOF:    return "SCANEOF";
        default:         return "UNKNOWN";
    }
}

static void synchronize_statement(void)
{
    while (current_token != SEMICOLON &&
           current_token != END &&
           current_token != SCANEOF) {

        current_token = scanner();
    }
}


void syntax_error(token actual)
{
    syntax_errors++;

    fprintf(stderr,
            "Error sintactico: token inesperado %s.\n",
            token_name(actual));
}


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

   /*
    * Error recovery:
    * consume the unexpected token so the parser
    * can continue instead of getting stuck
    */
   if (current_token != SCANEOF) {
       current_token = scanner();
   }
}



void system_goal(void)
{
    program();
    match(SCANEOF);
}


void program(void)
{
   if (current_token != BEGIN) {
       syntax_error(current_token);

       /*
        * If BEGIN is missing, recover until we find
        * a possible beginning of a statement or END
        */
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


void statement(void)
{
   switch (current_token) {

       case ID:
           match(ID);

           if (current_token != ASSIGNOP) {
               syntax_error(current_token);
               synchronize_statement();

               if (current_token == SEMICOLON) {
                   match(SEMICOLON);
               }

               return;
           }

           match(ASSIGNOP);
           expression();
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



void id_list(void)
{
    match(ID);

    while (current_token == COMMA) {
        match(COMMA);
        match(ID);
    }
}


void expr_list(void)
{
    expression();

    while (current_token == COMMA) {
        match(COMMA);
        expression();
    }
}
