%option noyywrap yylineno

%{

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "parser.tab.h"

#define YY_USER_ACTION \
    yylloc.first_line = yylineno; \
    yylloc.last_line = yylineno; \
    yylloc.first_column = 1; \
    yylloc.last_column = yyleng;

int lexical_error= 0;

%}
%%

[ \t\r\n]+       ;



"/*"([^*]|\*+[^*/])*\*+"/"    ;
"//".*          ;

"int"          {return INT;}
"float"        {return FLOAT;}
"char"         {return CHAR;}
"string"       {return STRING;}
"bool"         {return BOOL;}
"if"           {return IF;}
"else"         {return ELSE;}
"do"           {return DO;}
"while"        {return WHILE;}
"for"          {return FOR;}
"print"        {return PRINT;}
"input"        {return INPUT;} 
"true"         {return TRUE;}
"false"        {return FALSE;}

[0-9]+\.[0-9]+\.[0-9]+ {
    lexical_error=1;
    printf("Lexical Error at line %d: Malformed numeric literal '%s'\n",
           yylineno, yytext);
}

[0-9]+(\.[0-9]+)*[a-zA-Z_][a-zA-Z0-9_]* {
    lexical_error=1;
    printf("Lexical Error at line %d: Malformed numeric literal '%s'\n",
           yylineno, yytext);
}

[0-9]+"."[0-9]+ {
    yylval.text= strdup(yytext);
    return FLOAT_LITERAL;}

[0-9]+         {
    yylval.text= strdup(yytext);
    return INT_LITERAL;}

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.text= strdup(yytext);
    return IDENTIFIER;}

"=="    { return EQ; }
"!="    { return NE; }
"<="    { return LE; }
">="    { return GE; }
"<"     { return LT; }
">"     { return GT; }
"&&"    { return AND; }
"||"    { return OR; }
"!"     { return NOT; }
"="     { return ASSIGN; }
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return MULTIPLY; }
"/"     { return DIVIDE; }
"%"     { return MODULO; }


"("    { return LPAREN; }
")"    { return RPAREN; }
"{"    { return LBRACE; }
"}"    { return RBRACE; }
";"    { return SEMICOLON; }
","    { return COMMA; }


\"([^\\\"\n]|\\.)*\"    {
    yylval.text = (char*)malloc(yyleng - 1);
    strncpy(yylval.text, yytext + 1, yyleng - 2);
    yylval.text[yyleng - 2] = '\0';
    return STRING_LITERAL; }

\"([^\\\"\n]|\\.)*\n    {
    lexical_error=1;
    printf("Lexical Error at line %d: Unterminated string literal\n",
     yylineno);
}

\"([^\\\"\n]|\\.)* {
    lexical_error = 1;
    printf("Lexical Error at line %d: Unterminated string literal\n",
           yylineno);
}


. {
    lexical_error = 1;
    printf("Lexical Error at line %d: Illegal Character '%s'\n",
           yylineno, yytext);
}

%%
