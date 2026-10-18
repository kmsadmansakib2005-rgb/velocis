%option noyywrap yylineno

%{

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "parser.tab.h"

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

[0-9]+"."[0-9]+ {return FLOAT_LITERAL;}
[0-9]+         {return INT_LITERAL;}

[a-zA-Z_][a-zA-Z0-9_]*  {return IDENTIFIER;}

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


\"([^\\\"\n]|\\.)*\"    { return STRING_LITERAL; }

\"([^\\\"\n]|\\.)*\n    {
    printf("Lexical Error at line %d: Unterminated string literal\n", yylineno);
}


.           {
    printf("Lexical Error at line %d: Illegal Charecter '%s'\n",yylineno,yytext);

}

%%
