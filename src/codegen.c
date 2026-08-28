#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

/* Output assembly file */
static FILE *output_file = NULL;

/* Number of temporary variables */
static int temp_count = 0;
static int label_count = 0;

int codegen_is_active(void)
{
        return output_file != NULL;
}

/* Generate the data section */
static void generate_data_section(void)
{
        int i;

        fprintf(output_file, "\n.section .data\n");

        fprintf(output_file,
                "input_format: .string \"%%d\"\n");

        fprintf(output_file,
                "output_format: .string \"%%d\\n\"\n");

        for (i = 0; i < symtab_count(); i++) {
                fprintf(output_file,
                        "%s: .long 0\n",
                        symtab_name(i));
        }
}

/* Start code generation */
void codegen_init(const char *filename)
{
        output_file = fopen(filename, "w");

        if (output_file == NULL) {
                fprintf(stderr,
                        "Error: could not create assembly file\n");

                exit(EXIT_FAILURE);
        }

        temp_count = 0;

        fprintf(output_file,
                ".section .text\n");

        fprintf(output_file,
                ".globl main\n");

        fprintf(output_file,
                ".extern printf\n");

        fprintf(output_file,
                ".extern scanf\n");

        fprintf(output_file,
                "main:\n");

        fprintf(output_file,
                "    pushq %%rbp\n");

        fprintf(output_file,
                "    movq %%rsp, %%rbp\n");
}

/* Finish code generation */
void codegen_end(void)
{
        fprintf(output_file,
                "\n    movl $0, %%eax\n");

        fprintf(output_file,
                "    popq %%rbp\n");

        fprintf(output_file,
                "    ret\n");

        generate_data_section();

        /* Mark as non-executable to silence linker warning */
        fprintf(output_file,
                "\n.section .note.GNU-stack,\"\",@progbits\n");
}

/* Close assembly file */
void codegen_close(void)
{
        if (output_file != NULL) {
                fclose(output_file);
                output_file = NULL;
        }
}

/* Process an identifier */
expr_rec process_id(const char *name)
{
        expr_rec result;

        /* Register identifier in symbol table */
        check_id(name);

        result.kind = IDEXPR;

        strncpy(result.name, name, MAXIDLEN - 1);
        result.name[MAXIDLEN - 1] = '\0';

        return result;
}

/* Process an integer literal */
expr_rec process_lit(int value)
{
        expr_rec result;
        result.kind = LITERALEXPR;
        result.val = value;
        return result;
}

/* Generate a new temporary variable */
char *get_temp(void)
{
        static char temp_name[MAXIDLEN];

        temp_count++;

        snprintf(temp_name,
                sizeof(temp_name),
                "_temp%d",
                temp_count);

        check_id(temp_name);

        return temp_name;
}

/* Generate assignment code */
void assign(expr_rec target, expr_rec source)
{
        if (output_file == NULL) {
        return;
        }

        if (source.kind == LITERALEXPR) {

                fprintf(output_file,
                        "    movl $%d, %%eax\n",
                        source.val);

        } else {

                fprintf(output_file,
                        "    movl %s(%%rip), %%eax\n",
                        source.name);
        }

        fprintf(output_file,
                "    movl %%eax, %s(%%rip)\n",
                target.name);
}

/* Generate code for addition and subtraction */
expr_rec generate_infix(expr_rec left, token op, expr_rec right)
{
        expr_rec result;
        char *temp;
        temp = get_temp();
        result.kind = TEMPEXPR;
        strncpy(result.name, temp, MAXIDLEN - 1);
        result.name[MAXIDLEN - 1] = '\0';

        if (output_file == NULL) {
        return result;
        }

        /* Load left operand into EAX */
        if (left.kind == LITERALEXPR) {
                fprintf(output_file,
                        "    movl $%d, %%eax\n",
                        left.val);

        } else {
                fprintf(output_file,
                        "    movl %s(%%rip), %%eax\n",
                        left.name);
        }

        /* Apply operator */
        if (op == PLUSOP) {

                if (right.kind == LITERALEXPR) {

                fprintf(output_file,
                        "    addl $%d, %%eax\n",
                        right.val);

                } else {

                fprintf(output_file,
                        "    addl %s(%%rip), %%eax\n",
                        right.name);
                }

        } else if (op == MINUSOP) {

                if (right.kind == LITERALEXPR) {

                fprintf(output_file,
                        "    subl $%d, %%eax\n",
                        right.val);

                } else {

                fprintf(output_file,
                        "    subl %s(%%rip), %%eax\n",
                        right.name);
                }

        } else {

                fprintf(stderr,
                        "Error interno: operador no soportado\n");
                exit(EXIT_FAILURE);
        }

        /* Store result in temporary */
        fprintf(output_file,
                "    movl %%eax, %s(%%rip)\n",
                result.name);
        return result;
}

/* Load an expr_rec's value into %eax */
static void load_eax(expr_rec value)
{
        if (value.kind == LITERALEXPR) {
                fprintf(output_file,
                        "    movl $%d, %%eax\n",
                        value.val);
        } else {
                fprintf(output_file,
                        "    movl %s(%%rip), %%eax\n",
                        value.name);
        }
}

/* Generate code for conditional expression */
expr_rec generate_conditional(expr_rec cond, expr_rec true_val, expr_rec false_val)
{
        expr_rec result;
        char *temp;
        int label_id;
        temp = get_temp();
        result.kind = TEMPEXPR;
        strncpy(result.name, temp, MAXIDLEN - 1);
        result.name[MAXIDLEN - 1] = '\0';

        if (output_file == NULL) {
                return result;
        }

        label_id = ++label_count;

        /* Evaluate condition */
        load_eax(cond);

        fprintf(output_file,
                "    cmpl $0, %%eax\n");

        fprintf(output_file,
                "    je .Lcond_else%d\n",
                label_id);

        /* True branch */
        load_eax(true_val);

        fprintf(output_file,
                "    movl %%eax, %s(%%rip)\n",
                result.name);

        fprintf(output_file,
                "    jmp .Lcond_end%d\n",
                label_id);

        /* False branch */
        fprintf(output_file,
                ".Lcond_else%d:\n",
                label_id);

        load_eax(false_val);

        fprintf(output_file,
                "    movl %%eax, %s(%%rip)\n",
                result.name);

        fprintf(output_file,
                ".Lcond_end%d:\n",
                label_id);

        return result;
}

/* Generate READ code */
void read_id(expr_rec variable)
{
        fprintf(output_file,
                "    leaq input_format(%%rip), %%rdi\n");

        fprintf(output_file,
                "    leaq %s(%%rip), %%rsi\n",
                variable.name);

        fprintf(output_file,
                "    movl $0, %%eax\n");

        fprintf(output_file,
                "    call scanf@PLT\n");
}

/* Generate WRITE code */
void write_expr(expr_rec expression)
{
        fprintf(output_file,
                "    leaq output_format(%%rip), %%rdi\n");

        if (expression.kind == LITERALEXPR) {

                fprintf(output_file,
                        "    movl $%d, %%esi\n",
                        expression.val);

        } else {

                fprintf(output_file,
                        "    movl %s(%%rip), %%esi\n",
                        expression.name);
        }

        fprintf(output_file,
                "    movl $0, %%eax\n");

        fprintf(output_file,
                "    call printf@PLT\n");
}