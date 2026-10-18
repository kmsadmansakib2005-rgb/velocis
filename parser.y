%{
    #include<stdio.h>
    #include<stdlib.h>

    int yylex(void);
    void yyerror(const char *s);
%}

%token INT FLOAT CHAR STRING BOOL
%token DO WHILE FOR IF ELSE PRINT INPUT
%token TRUE FALSE

%token INT_LITERAL FLOAT_LITERAL STRING_LITERAL IDENTIFIER
%token EQ NE LE GE LT GT AND OR NOT ASSIGN
%token PLUS MINUS MULTIPLY DIVIDE MODULO

%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

%%

program:
        /* empty*/
        | program statement
        ;

statement:
        INT IDENTIFIER ASSIGN INT_LITERAL SEMICOLON
        |FLOAT IDENTIFIER ASSIGN FLOAT_LITERAL SEMICOLON
        |BOOL IDENTIFIER ASSIGN TRUE SEMICOLON
        |BOOL IDENTIFIER ASSIGN FALSE SEMICOLON
        |CHAR IDENTIFIER ASSIGN IDENTIFIER SEMICOLON
        |STRING IDENTIFIER ASSIGN IDENTIFIER SEMICOLON
        |PRINT LPAREN IDENTIFIER RPAREN SEMICOLON
        |PRINT LPAREN INT_LITERAL RPAREN SEMICOLON
        |PRINT LPAREN FLOAT_LITERAL RPAREN SEMICOLON
        |PRINT LPAREN TRUE RPAREN SEMICOLON
        |PRINT LPAREN FALSE RPAREN SEMICOLON
        ;

expression:
        INT_LITERAL
        |FLOAT_LITERAL
        |IDENTIFIER
        |expression PLUS expression
        |expression MINUS expression
        |expression DIVIDE expression
        |expression MULTIPLY expression
        |expression MODULO expression
        |PRINT LPAREN expression RPAREN
        ;



%%

void yyerror(const char *s)
{
    fprintf(stderr, "Syntax error: %s\n", s);
}

int main(void)
{
    printf("Velocis Compiler.\n");
    yyparse();

    return 0;
}