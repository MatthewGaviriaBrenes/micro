/*
 * Micro Compiler
 * File: symtab.c
 * Responsibility: Persona B
 *
 */

#include <stdio.h>
#include <string.h>

#include "symtab.h"

#define MAX_SYMBOLS 1024

/*
 * Cada entrada de la tabla representa un identificador
 * utilizado por el programa Micro.
 *
 * Como Micro solamente tiene variables integer y las
 * declaraciones son implícitas, por ahora solamente
 * necesitamos almacenar su nombre.
 */
typedef struct {
    char name[MAXIDLEN + 1];
} Symbol;

/* Tabla de símbolos */
static Symbol symbol_table[MAX_SYMBOLS];

/* Cantidad de identificadores almacenados */
static int symbol_count = 0;


void symtab_init(void)
{
    symbol_count = 0;
}


int lookup(const char *name)
{
    int i;

    if (name == NULL) {
        return 0;
    }

    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return 1;
        }
    }

    return 0;
}


int enter(const char *name)
{
    if (name == NULL) {
        return 0;
    }

    /* No insertar identificadores repetidos */
    if (lookup(name)) {
        return 0;
    }

    /* Evitar desbordamiento de la tabla */
    if (symbol_count >= MAX_SYMBOLS) {
        fprintf(stderr,
                "Error semantico: se alcanzo el maximo "
                "de simbolos permitidos.\n");
        return 0;
    }

    strncpy(symbol_table[symbol_count].name,
            name,
            MAXIDLEN);

    symbol_table[symbol_count].name[MAXIDLEN] = '\0';

    symbol_count++;

    return 1;
}


void check_id(const char *name)
{
    if (name == NULL) {
        return;
    }

    if (!lookup(name)) {
        enter(name);
    }
}