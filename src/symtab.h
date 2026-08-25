#ifndef SYMTAB_H
#define SYMTAB_H

#define MAXIDLEN 32

/* Check if an identifier exists */
int lookup(const char *name);

/* Add an identifier to the symbol table */
int enter(const char *name);

/* Add the identifier if it does not exist */
void check_id(const char *name);

/* Initialize the symbol table */
void symtab_init(void);

#endif