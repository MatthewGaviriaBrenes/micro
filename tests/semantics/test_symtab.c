#include <stdio.h>
#include <stdlib.h>

#include "../../src/symtab.h"

int main(void)
{
    symtab_init();

    printf("A existe: %d\n", lookup("A"));

    enter("A");

    printf("A existe: %d\n", lookup("A"));

    enter("B");

    printf("B existe: %d\n", lookup("B"));

    printf("C existe: %d\n", lookup("C"));

    check_id("C");

    printf("C existe despues de check_id: %d\n", lookup("C"));

    check_id("A");

    printf("A sigue existiendo: %d\n", lookup("A"));

    return EXIT_SUCCESS;
}