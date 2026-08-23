#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "scanner.h"

/*
 * Representa el resultado semántico de una expresión.
 */
typedef struct {
    int is_constant;
    int value;
} ExprRecord;

/*
 * Procesa un identificador.
 */
ExprRecord check_id_expr(const char *name);

/*
 * Procesa un literal entero.
 */
ExprRecord process_literal(const char *text);

/*
 * Genera/procesa una operación binaria.
 *
 * Si ambos operandos son constantes,
 * realiza constant folding.
 */
ExprRecord gen_infix(
    ExprRecord left,
    token op,
    ExprRecord right
);

#endif