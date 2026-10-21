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

        
       // printf("DEBUG: type=%d, value=%s\n", type,
        //  value == NULL ? "(NULL)" : value);

        node->type= type;
        node->value = (value!=NULL) ? strdup(value): NULL;

        if(node->value==NULL && value!=NULL)
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

int ast_evaluate(const ASTNode* node)
{
    if(node==NULL)
    {
        printf("Error: Cannot evaluate and empty AST Mode.\n");
        exit(1);
    }    

        switch(node->type)
        {
            case AST_INT:
                return atoi(node->value);
                
            case AST_BINARY:
            {
                int left= ast_evaluate(node->left);
                int right= ast_evaluate(node->right);

                if(strcmp(node->value, "+")==0)
                {
                    return left + right;
                }
                else if(strcmp(node->value, "-")==0)
                {
                    return left - right;
                }
                else if(strcmp(node->value, "*")==0)
                {
                    return left * right;
                }
                else if(strcmp(node->value, "/")==0)
                {
                    if(right==0)
                    {
                        printf("Error: Division by Zero.\n");
                        exit(1);
                    }
                    return left / right;
                }
                 else if(strcmp(node->value, "%")==0)
                {
                    if(right==0)
                    {
                        printf("Error: Module by Zero.\n");                       
                        exit(1);
                    }
                    return left % right;
                }
                printf("Error: Unsupported Operator %s.\n", node->value);
                exit(1);
            }  
            default:  
                printf("Error: Unsupported AST Node type.\n");
                exit(1);
        }
    }