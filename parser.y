%{
    #include<stdio.h>
    #include<stdlib.h>
    #include "ast.h"

    ASTNode* root= NULL;

    int yylex(void);
    void yyerror(const char *s);
    extern int lexical_error;
%}

%code requires {
    typedef struct ASTNode ASTNode;
}

%locations

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
        $$= ast_create(AST_PROGRAM, NULL, NULL, NULL, 1);
        root= $$;
        }
        | program statement
        {
            $$= ast_create(AST_PROGRAM, NULL, $1, $2, @2.first_line);
            root= $$;
        }
        ;

statement:
        INT IDENTIFIER ASSIGN INT_LITERAL SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_INT, $4, NULL, NULL, @4.first_line),
             NULL, @1.first_line);

            free($2);
            free($4);
        }
        |FLOAT IDENTIFIER ASSIGN FLOAT_LITERAL SEMICOLON
        {
            
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_FLOAT, $4, NULL, NULL, @4.first_line),
              NULL, @1.first_line);
            
            free($2);
            free($4);
        }
        |BOOL IDENTIFIER ASSIGN TRUE SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_BOOL, "true", NULL, NULL, @4.first_line), NULL,
             @1.first_line);
            
            free($2);
        }

        |BOOL IDENTIFIER ASSIGN FALSE SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_BOOL, "false" , NULL, NULL, @4.first_line),
             NULL, @1.first_line);
            
            free($2);
           
        }

        |CHAR IDENTIFIER ASSIGN IDENTIFIER SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_CHAR, $4, NULL, NULL, @4.first_line),
              NULL, @1.first_line);
            
            free($2);
            free($4);
        }
        
        |STRING IDENTIFIER ASSIGN STRING_LITERAL SEMICOLON
        {
            $$=ast_create(AST_DECLARATION, $2,
             ast_create(AST_IDENTIFIER, $4, NULL, NULL, @4.first_line), 
             NULL,@1.first_line);
            
            free($2);
            free($4);
        }
        |PRINT LPAREN expression RPAREN SEMICOLON
        {
              $$ = ast_create(AST_PRINT, NULL, $3, NULL, @1.first_line);
        }
        |IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$= ast_create(AST_ASSIGNMENT, $1, NULL, $3, @1.first_line);
            
            free($1);
        }
        ;

expression:
        INT_LITERAL
        {
            $$= ast_create(AST_INT, $1, NULL, NULL, @1.first_line);
        }
        |FLOAT_LITERAL
        {
            $$= ast_create(AST_FLOAT, $1, NULL, NULL,  @1.first_line);
        }

        |IDENTIFIER
        {
            $$= ast_create(AST_IDENTIFIER, $1, NULL, NULL,  @1.first_line);
        }
        |expression PLUS expression
        {
            $$= ast_create(AST_BINARY,"+", $1, $3,@2.first_line);
        }
        |expression MINUS expression
        {
            $$= ast_create(AST_BINARY,"-", $1, $3, @2.first_line);
        }
        |expression DIVIDE expression
        {
            $$= ast_create(AST_BINARY,"/", $1, $3, @2.first_line);
        }
        |expression MULTIPLY expression
        {
            $$= ast_create(AST_BINARY,"*", $1, $3, @2.first_line);
        }
        |expression MODULO expression
        {
            $$= ast_create(AST_BINARY,"%", $1, $3, @2.first_line);
        }
        ;



%%

void yyerror(const char *s)
{
    if(!lexical_error)
    {
        fprintf(stderr, "Syntax error at line %d: %s\n",
        yylloc.first_line,s);
    }
}

int main(void)
{
    printf("Velocis Compiler.\n");

    int parse_status= yyparse();
    
    if(parse_status==0 && lexical_error==0)
    {
       if(ast_semantic_analysis(root))
       {
         ast_export(root);
         ast_execute(root);
       }
    }
    else if(lexical_error!=0)
    {
        printf("Compilation stopped due to lexical errors.\n");
    }
    
    ast_free(root);
    ast_free_variables();
    return 0;
}