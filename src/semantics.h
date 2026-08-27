#ifndef SEMANTICS_H
#define SEMANTICS_H

typedef struct {
    int is_constant;
    int value;
} SemanticValue;

/* Create a constant semantic value */
SemanticValue make_constant(int value);

/* Check and register an identifier */
void check_identifier(const char *name);

/* Generate an infix operation */
SemanticValue gen_infix(SemanticValue left,
                        int operator,
                        SemanticValue right);

#endif 