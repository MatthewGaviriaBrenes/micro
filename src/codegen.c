/*
 * Micro Compiler
 * File: codegen.c
 * Responsibility: Matthew
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

#define MAX_SYMBOLS 1024

//Symbol table
static char symbol_table[MAX_SYMBOLS][MAXIDLEN];
static int symbol_count = 0;

//Output x86 file
static FILE *output_file = NULL;

//count temporary variables
static int temp_count = 0;

//lookup in symbol table---------------------------------------
//1 if the symbol exists
//0 if it doesnt exist
static int symbol_exists(const char *name)
{
    int i;

    for (i = 0; i < symbol_count; i++) {

        if (strcmp(symbol_table[i], name) == 0) {
            return 1;
        }
    }

    return 0;
}

//adds symbol to table----------------------------------------
static void add_symbol(const char *name)
{
    if (symbol_exists(name)) {
        return;
    }

    if (symbol_count >= MAX_SYMBOLS) {
        fprintf(stderr,
                "Error: Exceed max num of symbols.\n");

        exit(EXIT_FAILURE);
    }

    strncpy(symbol_table[symbol_count],
            name,
            MAXIDLEN - 1);

    symbol_table[symbol_count][MAXIDLEN - 1] = '\0';

    symbol_count++;
}

// Generates .section .data with and variables-----------------
static void generate_data_section(void)
{
    int i;

    fprintf(output_file, "\n.section .data\n");

    fprintf(output_file, "input_format: .string \"%%d\"\n");
    fprintf(output_file, "output_format: .string \"%%d\\n\"\n");

    /*
     * Declare all identifiers and temporaries.
     */
    for (i = 0; i < symbol_count; i++) {

        fprintf(output_file,
                "%s: .long 0\n",
                symbol_table[i]);
    }
}

//codegen init-----------------------------------------------
void codegen_init(const char *filename)
{
    output_file = fopen(filename, "w");

    if(output_file == NULL){
        fprintf(stderr, 
            "ERROR: Assembly file could not be created.\n");
        exit(EXIT_FAILURE);
    }

    symbol_count = 0;
    temp_count = 0;


    //Begin the text section.
     
    fprintf(output_file, ".section .text\n");

    
    //Make main visible to the linker.
    
    fprintf(output_file, ".globl main\n");

    
    //Program entry point.
    
    fprintf(output_file, "main:\n");
}

//codegen end------------------------------------------------
void codegen_end(void)
{   
    //return 0 = end program
    fprintf(output_file, "\n    movl $0, %%eax\n");
    fprintf(output_file, "    ret\n");

    //.section .data  
    generate_data_section();
}
/*    
    generate_data_section();

    //.section .text     
    fprintf(output_file, "\n.section .text\n");
    fprintf(output_file, ".globl main\n");
    fprintf(output_file, "main:\n"); //Assembly main

    //return 0 = end program
    fprintf(output_file, "    movl $0, %%eax\n");
    fprintf(output_file, "    ret\n");


}*/

//Close output file------------------------------------------
void codegen_close(void)
{
    if (output_file != NULL) {
        fclose(output_file);
        output_file = NULL;
    }
}

//ID processing-----------------------------------------------
expr_rec process_id(const char *name)
{
    expr_rec result;

    //Register identifier
    add_symbol(name);

    //Build expression
    result.kind = IDEXPR;

    strncpy(result.name,
            name,
            MAXIDLEN - 1);

    result.name[MAXIDLEN - 1] = '\0';

    return result;
}



//lit processing---------------------------------------------
expr_rec process_lit(int value)
{
    expr_rec result;

    result.kind = LITERALEXPR;
    result.val = value;

    return result;
}

//Temporary generation---------------------------------------
char *get_temp(void)
{
    static char temp_name[MAXIDLEN];

    temp_count++;

    snprintf(temp_name,
             MAXIDLEN,
             "_temp%d",
             temp_count);

    add_symbol(temp_name);

    return temp_name;
}

//Assignment---------------------------------------------
void assign(expr_rec target, expr_rec source)
{
    //store results in target ID

    if (source.kind == LITERALEXPR) {

        fprintf(output_file,
        "    movl $%d, %%eax\n", source.val);

    } else {

       fprintf(output_file,
        "    movl %s, %%eax\n", source.name);
    }
    fprintf(output_file,
        "    movl %%eax, %s\n", target.name);
}

//inflix expression gen---------------------------------------
expr_rec gen_infix(expr_rec left, token op, expr_rec right)
{
    expr_rec result;
    char *temp;

    //new temporary forresult
    temp = get_temp();

    result.kind = TEMPEXPR;

    strncpy(result.name, temp, MAXIDLEN - 1);

    result.name[MAXIDLEN - 1] = '\0';

    //left operand in EAX.
    if (left.kind == LITERALEXPR) {

        fprintf(output_file,
                "    movl $%d, %%eax\n",left.val);

    } else {

        fprintf(output_file,
                "    movl %s, %%eax\n", left.name);
    }


    //do operation
    if (op == PLUSOP) {
        if (right.kind == LITERALEXPR) {

            fprintf(output_file,
                    "    addl $%d, %%eax\n", right.val);

        } else {

            fprintf(output_file,
                    "    addl %s, %%eax\n", right.name);
        }

    } else if (op == MINUSOP) {

        if (right.kind == LITERALEXPR) {

            fprintf(output_file,
                    "    subl $%d, %%eax\n", right.val);

        } else {
            fprintf(output_file,
                    "    subl %s, %%eax\n", right.name);
        }

    } else {

        fprintf(stderr,
                "Error interno: operator not available.\n");

        exit(EXIT_FAILURE);
    }


    //Store result in temporary.
    fprintf(output_file,
        "    movl %%eax, %s\n",
            result.name);

    return result;
}

//read gen---------------------------------------------------
void read_id(expr_rec variable)
{
    //address of input_format.
    fprintf(output_file,
            "    leaq input_format(%%rip), %%rdi\n");

    //address of the variable.
    fprintf(output_file,
            "    leaq %s(%%rip), %%rsi\n",
            variable.name);

    //Clear EAX
    fprintf(output_file,
            "    movl $0, %%eax\n");

    fprintf(output_file,
            "    call scanf@PLT\n");
}

//write gen-------------------------------------------------
void write_expr(expr_rec expression)
{
    //output_format adress
    fprintf(output_file,
            "    leaq output_format(%%rip), %%rdi\n");

    //int to print
    if (expression.kind == LITERALEXPR) {

        fprintf(output_file,
                "    movl $%d, %%esi\n",
                expression.val);

    } else {
        fprintf(output_file,
                "    movl %s, %%esi\n",
                expression.name);
    }

    // Clear EAX
    fprintf(output_file,
            "    movl $0, %%eax\n");

    fprintf(output_file,
            "    call printf@PLT\n");
}

