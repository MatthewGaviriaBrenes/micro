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

/* Symbol table entry */
typedef struct {
    char name[MAXIDLEN + 1];
} Symbol;

/* Symbol table storage */
static Symbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;


/* Initialize the symbol table */
void symtab_init(void)
{
    symbol_count = 0;
}

/* Search for an identifier */
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


/* Add a new identifier */
int enter(const char *name)
{
    if (name == NULL) {
        return 0;
    }

    /* Avoid duplicate identifiers */
    if (lookup(name)) {
        return 0;
    }

    /* Prevent table overflow */
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


/* Add the identifier if needed */
void check_id(const char *name)
{
    if (name == NULL) {
        return;
    }

    if (!lookup(name)) {
        enter(name);
    }
}

int symtab_count(void)
{
    return symbol_count;
}

const char *symtab_name(int index)
{
    if (index < 0 || index >= symbol_count) {
        return NULL;
    }

    return symbol_table[index].name;
}
