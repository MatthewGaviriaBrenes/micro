#ifndef CODEGEN_H
#define CODEGEN_H

#include "scanner.h"
#include "symtab.h"

//Expression types
typedef enum {
    IDEXPR,         //ID
    LITERALEXPR,    //int literal
    TEMPEXPR        //temporary expression 
} expr_kind;


//Expression record
typedef struct {
    expr_kind kind;

    union {
        char name[MAXIDLEN];  //ID or temporary
        int val;              //int literal
    };
} expr_rec;


//codegen interface

//Start code generation
void codegen_init(const char *filename);

//End code generation
void codegen_end(void);

void codegen_close(void);

//Process ID
expr_rec process_id(const char *name);

//Process int literal
expr_rec process_lit(int value);

//new temporary variable
char *get_temp(void);

// codegen for an assignment
void assign(expr_rec target, expr_rec source);

// codegen for add/sub
expr_rec generate_infix(expr_rec left, token op, expr_rec right);

//Code for READ
void read_id(expr_rec variable);

//Code for WRITE
void write_expr(expr_rec expression);

#endif