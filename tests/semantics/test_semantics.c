#include <stdio.h>
#include <stdlib.h>

#include "../../src/semantics.h"
#include "../../src/symtab.h"

int main(void)
{
    SemanticValue a;
    SemanticValue b;
    SemanticValue result;

    symtab_init();

    /* Test constant addition */
    a = make_constant(10);
    b = make_constant(20);

    result = gen_infix(a, '+', b);

    printf("10 + 20 = %d\n", result.value);
    printf("Constant: %d\n", result.is_constant);

    /* Test constant subtraction */
    a = make_constant(30);
    b = make_constant(5);

    result = gen_infix(a, '-', b);

    printf("30 - 5 = %d\n", result.value);
    printf("Constant: %d\n", result.is_constant);

    /* Test non-constant expression */
    a.is_constant = 0;
    a.value = 0;

    b = make_constant(10);

    result = gen_infix(a, '+', b);

    printf("Variable + 10 constant: %d\n",
            result.is_constant);

    return EXIT_SUCCESS;
}