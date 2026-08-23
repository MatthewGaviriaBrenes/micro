/*
* Micro Compiler
* File: semantics.c
* Responsibility: Persona B
*
*/

#include <stdlib.h>

#include "semantics.h"
#include "symtab.h"


ExprRecord check_id_expr(const char *name)
{
    ExprRecord result;

    /*
     * En Micro los identificadores se declaran
     * implícitamente.
     */
    check_id(name);

    /*
     * Un identificador no es una constante conocida
     * durante compilación.
     */
    result.is_constant = 0;
    result.value = 0;

    return result;
}


ExprRecord process_literal(const char *text)
{
    ExprRecord result;

    result.is_constant = 1;
    result.value = atoi(text);

    return result;
}


ExprRecord gen_infix(
    ExprRecord left,
    token op,
    ExprRecord right
)
{
    ExprRecord result;

    /*
     * CONSTANT FOLDING
     *
     * Si ambos lados son constantes, calculamos
     * el resultado durante compilación.
     */
    if (left.is_constant && right.is_constant) {

        result.is_constant = 1;

        switch (op) {

            case PLUSOP:
                result.value = left.value + right.value;
                break;

            case MINUSOP:
                result.value = left.value - right.value;
                break;

            default:
                result.value = 0;
                break;
        }

        return result;
    }

    /*
     * Si al menos uno de los operandos depende
     * de una variable, no podemos conocer el
     * resultado en tiempo de compilación.
     */
    result.is_constant = 0;
    result.value = 0;

    return result;
}