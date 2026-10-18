#include<stdio.h>

int yyparse(void);

int main(void)
{
    printf("Velocis Compiler v0.1\n");
    printf("parsing Velocis source code......\n\n");

    if(yyparse()==0)
    {
        printf("\nParsing successfull....!\n");
        return 0;
    }

    printf("\nParsing failed...!\n\n");
    return 1;


}