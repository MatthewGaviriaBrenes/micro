/*
 * Micro Compiler
 * File: semantics.c
 * Responsibility: Persona B
 *
 */

#include <stdio.h>
#include "semantics.h"
#include "symtab.h"

/* Create a constant semantic value */
SemanticValue make_constant(int value)
{
    SemanticValue result;
    result.is_constant = 1;
    result.value = value;
    return result;
}

/* Check and register an identifier */
void check_identifier(const char *name)
{
    check_id(name);
}

/* Generate an infix operation */
SemanticValue gen_infix(SemanticValue left,
                        int operator,
                        SemanticValue right)
{
    SemanticValue result;
    result.is_constant = 0;
    result.value = 0;

    if (left.is_constant && right.is_constant) {
        result.is_constant = 1;
        switch (operator) {
            case '+':
                result.value = left.value + right.value;
                break;
            case '-':
                result.value = left.value - right.value;
                break;
            default:
                fprintf(stderr,
                        "Error semantico: operador invalido\n");
                result.is_constant = 0;
                break;
        }
    }
    return result;
}