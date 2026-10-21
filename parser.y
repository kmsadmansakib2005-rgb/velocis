%{
    #include<stdio.h>
    #include<stdlib.h>
    #include "ast.h"

    ASTNode* root= NULL;

    int yylex(void);
    void yyerror(const char *s);
%}

%code requires {
    typedef struct ASTNode ASTNode;
}

%union
    {
        ASTNode* node;
        char* text;
    }

%token INT FLOAT CHAR STRING BOOL
%token DO WHILE FOR IF ELSE PRINT INPUT
%token TRUE FALSE

%token <text> INT_LITERAL FLOAT_LITERAL STRING_LITERAL IDENTIFIER
%type <node> program expression statement

%token EQ NE LE GE LT GT AND OR NOT ASSIGN
%token PLUS MINUS MULTIPLY DIVIDE MODULO

%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO

%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

%%

program:
        {
        $$= ast_create(AST_PROGRAM, NULL, NULL, NULL);
        root= $$;
        }
        | program statement
        {
            $$= ast_create(AST_PROGRAM, NULL, $1, $2);
            root= $$;
        }
        ;

statement:
        INT IDENTIFIER ASSIGN INT_LITERAL SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_INT, $4, NULL, NULL), NULL);
            
            ast_set_variable($2, atoi($4));

            free($2);
            free($4);
        }
        |FLOAT IDENTIFIER ASSIGN FLOAT_LITERAL SEMICOLON
        {
            
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_FLOAT, $4, NULL, NULL), NULL);
            
            free($2);
            free($4);
        }
        |BOOL IDENTIFIER ASSIGN TRUE SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_BOOL, "true", NULL, NULL), NULL);
            
            free($2);
        }

        |BOOL IDENTIFIER ASSIGN FALSE SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_BOOL, "false" , NULL, NULL), NULL);
            
            free($2);
           
        }

        |CHAR IDENTIFIER ASSIGN IDENTIFIER SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_CHAR, $4, NULL, NULL), NULL);
            
            free($2);
            free($4);
        }
        
        |STRING IDENTIFIER ASSIGN IDENTIFIER SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_IDENTIFIER, $4, NULL, NULL), NULL);
            
            free($2);
            free($4);
        }
        |PRINT LPAREN expression RPAREN SEMICOLON
        {
              $$ = ast_create(AST_PRINT, NULL, $3, NULL);
               printf("%d\n", ast_evaluate($3));
        }
        ;

expression:
        INT_LITERAL
        {
            $$= ast_create(AST_INT, $1, NULL, NULL);
        }
        |FLOAT_LITERAL
        {
            $$= ast_create(AST_FLOAT, $1, NULL, NULL);
        }

        |IDENTIFIER
        {
            $$= ast_create(AST_IDENTIFIER, $1, NULL, NULL);
        }
        |expression PLUS expression
        {
            $$= ast_create(AST_BINARY,"+", $1, $3);
        }
        |expression MINUS expression
        {
            $$= ast_create(AST_BINARY,"-", $1, $3);
        }
        |expression DIVIDE expression
        {
            $$= ast_create(AST_BINARY,"/", $1, $3);
        }
        |expression MULTIPLY expression
        {
            $$= ast_create(AST_BINARY,"*", $1, $3);
        }
        |expression MODULO expression
        {
            $$= ast_create(AST_BINARY,"%", $1, $3);
        }
        ;



%%

void yyerror(const char *s)
{
    fprintf(stderr, "Syntax error: %s\n", s);
}

int main(void)
{
    printf("Velocis Compiler.\n");
    if(yyparse()==0)
    {
        printf("\nAbstract Syntax Tree\n");
        ast_print(root, 0);
      
    }
    
    ast_free(root);
    return 0;
}