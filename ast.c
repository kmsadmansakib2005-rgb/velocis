#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "ast.h"

ASTNode *ast_create(ASTNodeType type, const char *value, 
    ASTNode* left,ASTNode* right)
    {
        ASTNode* node= malloc(sizeof(ASTNode));

        if(node==NULL)
        {
            printf("Memory Allocation failed.\n");
            exit(1);
        }

        node->type= type;
        node->value = strdup(value);

        if(node->value==NULL)
        {
            free(node);
            printf("Memory Allocation Failed!\n");
            exit(1);
        }

        node->left= left;
        node->right= right;

        return node;
    }

    void ast_print(const ASTNode* node, int depth)
    {
        if(node==NULL)
        {
            return;
        }

        for(int i=0; i<depth; i++)
        {
            printf("  ");
        }

        switch (node->type)
        {
            case AST_PROGRAM:
                printf("PROGRAM\n");
                break;

            case AST_INT:
                printf("INTEGER (%s)\n", node->value);
                break;

            case AST_FLOAT:
                printf("FLOAT (%s)\n", node->value)    ;
                break;

            case AST_IDENTIFIER:
                printf("IDENTIFIER (%s)\n", node->value);
                break;

            case AST_BOOL:    
                printf("BOOL (%s)\n", node->value);
                break;

            case AST_BINARY:
                printf("BINARY (%s)\n", node->value);
                break;
            
            case AST_PRINT:
                printf("PRINT\n");
                break;
    
            case AST_DECLARATION:
                 printf("DECLARATION (%s)\n", node->value);
                 break;    
        }

        ast_print(node->left, depth+1);
        ast_print(node->right, depth+1);
    }    

void ast_free(ASTNode *node)
{
    if(node== NULL)
    {
        return;
    }
    ast_free(node->left);
    ast_free(node->right);

    free(node->value);
    free(node);
}    