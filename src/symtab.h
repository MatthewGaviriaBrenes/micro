#ifndef SYMTAB_H
#define SYMTAB_H

#define MAXIDLEN 32

/*
 * Busca un identificador en la tabla de símbolos.
 *
 * Retorna:
 *   1 si el identificador existe.
 *   0 si no existe.
 */
int lookup(const char *name);

/*
 * Introduce un identificador en la tabla de símbolos.
 *
 * Retorna:
 *   1 si se agregó correctamente.
 *   0 si ya existía o no se pudo agregar.
 */
int enter(const char *name);

/*
 * Verifica que el identificador exista.
 * Si no existe, lo agrega.
 *
 * Esto corresponde a las declaraciones implícitas
 * del lenguaje Micro.
 */
void check_id(const char *name);

/*
 * Inicializa la tabla de símbolos.
 */
void symtab_init(void);

#endif