
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


static Variable* variables = NULL;



ASTNode* ast_create(ASTNodeType type, const char* value,
                    ASTNode* left, ASTNode* right)
{
    ASTNode* node = malloc(sizeof(ASTNode));

    if(node == NULL)
    {
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }

    node->type = type;
    node->value = value != NULL ? strdup(value) : NULL;
    node->left = left;
    node->right = right;

    if(value != NULL && node->value == NULL)
    {
        free(node);
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }

    return node;
}


/* Print the AST */
void ast_print(const ASTNode* node, int depth)
{
    if(node == NULL)
        return;

    for(int i = 0; i < depth; i++)
        printf("  ");

    switch(node->type)
    {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            break;

        case AST_INT:
            printf("INTEGER: %s\n", node->value);
            break;

        case AST_FLOAT:
            printf("FLOAT: %s\n", node->value);
            break;

        case AST_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->value);
            break;

        case AST_BINARY:
            printf("BINARY: %s\n", node->value);
            break;

        case AST_BOOL:
            printf("BOOLEAN: %s\n", node->value);
            break;

        case AST_PRINT:
            printf("PRINT\n");
            break;

        case AST_DECLARATION:
            printf("DECLARATION: %s\n", node->value);
            break;

        case AST_CHAR:
            printf("CHARACTER: %s\n", node->value);
            break;

        case AST_ASSIGNMENT:
            printf("ASSIGNMENT: %s\n", node->value);
            break;

        default:
            printf("UNKNOWN NODE\n");
            break;
    }

    ast_print(node->left, depth + 1);
    ast_print(node->right, depth + 1);
}



void ast_free(ASTNode* node)
{
    if(node == NULL)
        return;

    ast_free(node->left);
    ast_free(node->right);

    free(node->value);
    free(node);
}


static Variable* find_variable(const char* name)
{
    Variable* current = variables;

    while(current != NULL)
    {
        if(strcmp(current->name, name) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}



void ast_set_variable(const char* name, DataType type,
                      const char* value)
{
    if(name == NULL || value == NULL)
    {
        printf("Error: Invalid variable value!\n");
        exit(1);
    }

    Variable* current = find_variable(name);

    if(current == NULL)
    {
        current = malloc(sizeof(Variable));

        if(current == NULL)
        {
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }

        current->name = strdup(name);

        if(current->name == NULL)
        {
            free(current);
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }

        current->next = variables;
        variables = current;
    }
    else if(current->type == TYPE_STRING)
    {
        free(current->value.string_value);
        current->value.string_value = NULL;
    }

    current->type = type;

    switch(type)
    {
        case TYPE_INT:
            current->value.int_value = atoi(value);
            break;

        case TYPE_FLOAT:
            current->value.float_value = (float)atof(value);
            break;

        case TYPE_BOOL:
            current->value.bool_value = (strcmp(value, "true") == 0);
            break;

        case TYPE_CHAR:
            current->value.char_value = value[0];
            break;

        case TYPE_STRING:
            current->value.string_value = strdup(value);

            if(current->value.string_value == NULL)
            {
                printf("Error: Memory allocation failed!\n");
                exit(1);
            }
            break;
    }
}

Variable* ast_get_variable(const char* name)
{
    Variable* variable = find_variable(name);

    if(variable == NULL)
    {
        printf("Error: Undefined Variable: %s\n", name);
        exit(1);
    }

    return variable;
}


int ast_evaluate(const ASTNode* node)
{
    if(node == NULL)
    {
        printf("Error: Cannot evaluate an empty AST node.\n");
        exit(1);
    }

    switch(node->type)
    {
        case AST_INT:
            return atoi(node->value);

        case AST_IDENTIFIER:
        {
            Variable* variable = ast_get_variable(node->value);

            if(variable->type != TYPE_INT)
            {
                printf("Error: Expected an integer variable: %s\n",
                       node->value);
                exit(1);
            }

            return variable->value.int_value;
        }

        case AST_ASSIGNMENT:
        {
            int result = ast_evaluate(node->right);
            Variable* variable = ast_get_variable(node->value);

            if(variable->type != TYPE_INT)
            {
                printf("Error: Integer assignment requires an integer "
                       "variable: %s\n", node->value);
                exit(1);
            }

            char text[32];
            snprintf(text, sizeof(text), "%d", result);

            ast_set_variable(node->value, TYPE_INT, text);

            return result;
        }

        case AST_BINARY:
        {
            int left = ast_evaluate(node->left);
            int right = ast_evaluate(node->right);

            if(strcmp(node->value, "+") == 0)
                return left + right;

            if(strcmp(node->value, "-") == 0)
                return left - right;

            if(strcmp(node->value, "*") == 0)
                return left * right;

            if(strcmp(node->value, "/") == 0)
            {
                if(right == 0)
                {
                    printf("Error: Division by zero!\n");
                    exit(1);
                }

                return left / right;
            }

            if(strcmp(node->value, "%") == 0)
            {
                if(right == 0)
                {
                    printf("Error: Modulo by zero!\n");
                    exit(1);
                }

                return left % right;
            }

            printf("Error: Unsupported operator: %s\n", node->value);
            exit(1);
        }

        default:
            printf("Error: Unsupported expression type!\n");
            exit(1);
    }

    return 0;
}


void ast_execute(const ASTNode* node)
{
    if(node == NULL)
        return;

    switch(node->type)
    {
        case AST_PROGRAM:
            ast_execute(node->left);
            ast_execute(node->right);
            break;

        case AST_DECLARATION:
        {
            if(node->left == NULL || node->value == NULL)
            {
                printf("Error: Invalid declaration!\n");
                exit(1);
            }

            DataType type;
            const char* value = node->left->value;

            switch(node->left->type)
            {
                case AST_INT:
                    type = TYPE_INT;
                    break;

                case AST_FLOAT:
                    type = TYPE_FLOAT;
                    break;

                case AST_BOOL:
                    type = TYPE_BOOL;
                    break;

                case AST_CHAR:
                    type = TYPE_CHAR;
                    break;

                case AST_IDENTIFIER:

                    type = TYPE_STRING;
                    break;

                default:
                    printf("Error: Unsupported declaration type!\n");
                    exit(1);
            }

            ast_set_variable(node->value, type, value);
            break;
        }

        case AST_ASSIGNMENT:
            ast_evaluate(node);
            break;

        case AST_PRINT:
        {
            if(node->left == NULL)
                break;

            if(node->left->type == AST_FLOAT)
            {
                printf("%g\n", atof(node->left->value));
            }
            else if(node->left->type == AST_BOOL)
            {
                printf("%s\n", node->left->value);
            }
            else if(node->left->type == AST_CHAR)
            {
                printf("%c\n", node->left->value[0]);
            }
            else if(node->left->type == AST_IDENTIFIER)
            {
                Variable* variable =
                    ast_get_variable(node->left->value);

                switch(variable->type)
                {
                    case TYPE_INT:
                        printf("%d\n", variable->value.int_value);
                        break;

                    case TYPE_FLOAT:
                        printf("%g\n", variable->value.float_value);
                        break;

                    case TYPE_BOOL:
                        printf("%s\n",
                               variable->value.bool_value ? "true" : "false");
                        break;

                    case TYPE_CHAR:
                        printf("%c\n", variable->value.char_value);
                        break;

                    case TYPE_STRING:
                        printf("%s\n", variable->value.string_value);
                        break;
                }
            }
            else
            {
                printf("%d\n", ast_evaluate(node->left));
            }

            break;
        }

        default:
            break;
    }
}


/* Free all runtime variables */
void ast_free_variables(void)
{
    Variable* current = variables;

    while(current != NULL)
    {
        Variable* next = current->next;

        if(current->type == TYPE_STRING)
            free(current->value.string_value);

        free(current->name);
        free(current);

        current = next;
    }

    variables = NULL;
}


/* Export AST */
void ast_export(const ASTNode* node)
{
    printf("----AST START----\n");
    ast_print(node, 0);
    printf("\n---AST END----\n");
}
