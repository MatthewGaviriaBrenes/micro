#include <stdio.h>

#include "../../src/parser.h"


void expression(void)
{
    if (current_token == ID) {
        match(ID);
    }
    else if (current_token == INTLITERAL) {
        match(INTLITERAL);
    }
    else {
        syntax_error(current_token);
    }
}


void primary(void)
{
    /* Implemented by Persona B */
}


void add_op(void)
{
    /* Implemented by Persona B */
}
