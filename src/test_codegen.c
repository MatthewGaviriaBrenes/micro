#include "codegen.h"

int main(void)
{
    expr_rec a;
    expr_rec b;
    expr_rec ten;
    expr_rec five;
    expr_rec result;

    codegen_init("test.s");

    a = process_id("A");
    b = process_id("B");

    /* A := 10 */
    ten = process_lit(10);
    assign(a, ten);

    /* B := A + 5 */
    five = process_lit(5);
    result = gen_infix(a, PLUSOP, five);
    assign(b, result);

    /* write(B) */
    write_expr(b);

    codegen_end();
    codegen_close();

    return 0;
}