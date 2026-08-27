#ifndef SYMTAB_H
#define SYMTAB_H

#define MAXIDLEN 33
#define MAX_SYMBOLS 1024

/* Initialize the symbol table */
void symtab_init(void);

/* Check if an identifier exists */
int lookup(const char *name);

/* Add an identifier to the symbol table */
int enter(const char *name);

/* Add the identifier if it does not exist */
void check_id(const char *name);

int symtab_count(void);

const char *symtab_name(int index);

#endif 