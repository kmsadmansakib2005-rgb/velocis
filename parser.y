%{
   
    #include<stdio.h>
    #include<string.h>
    #include<stdlib.h>
    #include "ast.h"

    ASTNode* root= NULL;

    int syntax_error=0;

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
%type <node> program expression statement block block_statement

%token EQ NE LE GE LT GT AND OR NOT ASSIGN
%token PLUS MINUS MULTIPLY DIVIDE MODULO

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right NOT
%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE

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

        INT IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$ = ast_create(AST_DECLARATION, $2, $4, NULL, @1.first_line);
            $$->data_type= strdup("int");
            free($2);
        }

        | FLOAT IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$ = ast_create(AST_DECLARATION, $2, $4, NULL, @1.first_line);
            $$->data_type= strdup("float");
            free($2);
        }

        | BOOL IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$ = ast_create(AST_DECLARATION, $2, $4, NULL, @1.first_line);
            $$->data_type= strdup("bool");
            free($2);
        }

        | CHAR IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$ = ast_create(AST_DECLARATION, $2, $4, NULL, @1.first_line);
            $$->data_type= strdup("char");
            free($2);
        }

        | STRING IDENTIFIER ASSIGN expression SEMICOLON
        {
            $$ = ast_create(AST_DECLARATION, $2, $4, NULL, @1.first_line);
            $$->data_type= strdup("string");
            free($2);
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
        |IF LPAREN expression RPAREN block %prec IF_WITHOUT_ELSE
        {
            $$= ast_create(AST_IF, NULL, $3, $5, @1.first_line);
        }
        |IF LPAREN expression RPAREN block ELSE block
        {
            $$= ast_create(AST_IF, NULL, $3, $5, @1.first_line);
            $$->third= $7;
        }
        |DO block WHILE LPAREN expression RPAREN SEMICOLON
        {
            $$= ast_create(AST_DO_WHILE, NULL, $2, $5, @1.first_line);

        }
        |error SEMICOLON
        {
            syntax_error=1;
            yyerror;
            $$=NULL;
        }
        ;

block:
        LBRACE block_statement RBRACE
        {
            $$ = $2;
        }
        ;
block_statement:
        {
            $$= ast_create(AST_PROGRAM, NULL, NULL, NULL, @$.first_line);  
        }
        | block_statement statement
        {
            $$= ast_create(AST_PROGRAM, NULL, $1, $2,@2.first_line);
        }
         
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
        |expression EQ expression
        {
            $$=ast_create(AST_BINARY, "==", $1, $3, @2.first_line);
        }
        |expression NE expression
        {
            $$=ast_create(AST_BINARY, "!=", $1, $3, @2.first_line);
        }
        |expression LT expression
        {
            $$=ast_create(AST_BINARY, "<", $1, $3, @2.first_line);
        }
        |expression GT expression
        {
            $$=ast_create(AST_BINARY, ">", $1, $3, @2.first_line);
        }
        |expression LE expression
        {
            $$=ast_create(AST_BINARY, "<=", $1, $3, @2.first_line);
        }
        |expression GE expression
        {
            $$=ast_create(AST_BINARY, ">=", $1, $3, @2.first_line);
        }
        |expression AND expression
        {
            $$=ast_create(AST_BINARY, "&&", $1, $3, @2.first_line);
        }
        |expression OR expression        {
            $$=ast_create(AST_BINARY, "||", $1, $3, @2.first_line);
        }
        |NOT expression
        {
            $$=ast_create(AST_UNARY, "!", $2, NULL, @1.first_line);
        }
        |TRUE
        {
            $$=ast_create(AST_BOOL, "true", NULL, NULL, @1.first_line);
        }
        |FALSE
        {
            $$=ast_create(AST_BOOL, "false", NULL, NULL, @1.first_line);
        }
        |LPAREN expression RPAREN
        {
            $$=$2;
        }
        |STRING_LITERAL
        {
            $$=ast_create(AST_STRING, $1, NULL, NULL, @1.first_line);
            free($1);
        }
        ;

%%

void yyerror(const char *s)
{
    if(!lexical_error)
    {
        syntax_error=1;
        fprintf(stderr, "Syntax error at line %d: %s\n",
        yylloc.first_line,s);
    }
}

int main(void)
{
    printf("Velocis Compiler.\n");

    int parse_status= yyparse();
    
    if(parse_status==0 && lexical_error==0 &&syntax_error==0)
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