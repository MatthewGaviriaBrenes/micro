#ifndef SCANNER_H
#define SCANNER_H

#define MAX_IDENTIFIER_LENGTH 32

typedef enum token_types {
    BEGIN,
    END,
    READ,
    WRITE,
    ID,
    INTLITERAL,
    LPAREN,
    RPAREN,
    SEMICOLON,
    COMMA,
    ASSIGNOP,
    PLUSOP,
    MINUSOP,
    SCANEOF
} token;

extern char token_buffer[];
extern int lexical_errors;

token scanner(void);

#endif
