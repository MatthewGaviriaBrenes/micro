#include <stdio.h>
#include <stdlib.h>

#include "../../src/semantics.h"
#include "../../src/symtab.h"

int main(void)
{
    ExprRecord a;
    ExprRecord b;
    ExprRecord result;

    symtab_init();

    /* 10 */
    a = process_literal("10");

    /* 20 */
    b = process_literal("20");

    /* 10 + 20 */
    result = gen_infix(a, PLUSOP, b);

    printf(
        "10 + 20 -> constante: %d, valor: %d\n",
        result.is_constant,
        result.value
    );

    /* 10 + 20 - 5 */
    b = process_literal("5");

    result = gen_infix(
        result,
        MINUSOP,
        b
    );

    printf(
        "30 - 5 -> constante: %d, valor: %d\n",
        result.is_constant,
        result.value
    );

    /* Identificador */
    result = check_id_expr("A");

    printf(
        "A -> constante: %d\n",
        result.is_constant
    );

    return EXIT_SUCCESS;
}