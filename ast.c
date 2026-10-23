
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


static Variable* variables = NULL;

typedef struct SemanticSymbol
{
    char* name;
    struct SemanticSymbol* next;
}SemanticSymbol;

static SemanticSymbol* semantic_symbol= NULL;



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

float ast_evaluate_float(const ASTNode* node)
{
    if(node== NULL)
    {
        fprintf(stderr, "Error: Cannot evaluate an empty expression.\n");
        return 0.0f;
    }

    switch (node->type)
    {
        case AST_FLOAT:
            return (float)atof(node->value);
        
        case AST_INT:
            return (float)atoi(node->value);
        
        case AST_IDENTIFIER:
        {
            Variable* variable= ast_get_variable(node->value);
            if(variable->type==TYPE_FLOAT)
                return variable->value.float_value;
            
            if(variable->type==TYPE_INT)    
                return (float)variable->value.int_value;
             
            printf("Error: Variable %s is not numeric.\n", node->value);
            exit(1);  
            
        } 
        case AST_ASSIGNMENT:
        {
            float result= ast_evaluate_float(node->right);
            Variable* variable=ast_get_variable(node->value);

            if(variable->type!= TYPE_FLOAT)
            {
                printf("Error: Float assignment requires an float variable: %s\n", node->value);
                exit(1);
            }
            char text[32];
            snprintf(text, sizeof(text), "%.9g", (double)result);
            ast_set_variable(node->value, TYPE_FLOAT, text);
            return result;
        }       
        case AST_BINARY:
        {
            float left= ast_evaluate_float(node->left);
            float right= ast_evaluate_float(node->right);

            
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
            printf("Error: Unsupported operator: %s", node->value)  ;
            exit(1);   
        }
        default:
            printf("Error: Unsupported operator: %s", node->value)  ;
            exit(1);
    }
    return 0.0f;
    
}

int ast_evaluate_float_expression(const ASTNode* node)
{
    if(node==NULL)
    {
        return 0;
    }
    if(node->type==AST_FLOAT)
    {
        return 1;
    }

    if(node->type==AST_IDENTIFIER)
    {
        Variable* variable= ast_get_variable(node->value);
        return variable->type== TYPE_FLOAT;
    }

    if(node->type== AST_BINARY)
    {
        return ast_evaluate_float_expression(node->left)||
               ast_evaluate_float_expression(node->right);
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
        {
            Variable* variable= ast_get_variable(node->value);

            if(variable->type==TYPE_INT)
            {
                ast_evaluate(node);
            }
            else if(variable->type==TYPE_FLOAT)
            {
                ast_evaluate_float(node);
            }
            break;
        }    
        

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
            else if (node->left->type == AST_BINARY)
            {
                
                if(ast_evaluate_float_expression(node->left))
                {
                    printf("%g\n", ast_evaluate_float(node->left));
                }
                else
                {
                    printf("%d\n", ast_evaluate(node->left));
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

void ast_export(const ASTNode* node)
{
    printf("----AST START----\n");
    ast_print(node, 0);
    printf("\n---AST END----\n");
}

static int is_declared(const ASTNode* node, const char* name)
{
    if(node==NULL)
    {
        return 0;
    }
    if(node->type==AST_PROGRAM)
    {
        if(is_declared(node->left, name))
        {
            return 1;
        }
        if(is_declared(node->right, name))
        {
            return 1;
        }
    }

    if(node->type==AST_DECLARATION && node->value!=NULL &&
        strcmp(node->value, name)==0)
        {
            return 1;
        }
    
    return 0;    
}

static int semantic_symbol_exists(const char* name)
{
    SemanticSymbol* current= semantic_symbol;

    while(current!=NULL)
    {
        if(strcmp(current->name, name)==0)
        {
            return 1;
        }
        current=current->next;
    }
    return 0;
}

static void semantic_add_symbol(const char* name)
{
    SemanticSymbol* symbol=malloc(sizeof(SemanticSymbol));

    if(symbol==NULL)
    {
        fprintf(stderr, "Sematic error: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    symbol->name= malloc(strlen(name)+1);
    if(symbol->name==NULL)
    {
        free(symbol);
        fprintf(stderr, "Semantic error: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(symbol->name, name);
    symbol->next= semantic_symbol;
    semantic_symbol=symbol;

}

static void semantic_clear_symbol(void)
{
    SemanticSymbol* current= semantic_symbol;
    while(current!=NULL)
    {
        SemanticSymbol* next=current->next;

        free(current->name);
        free(current);

        current=next;
    }
    semantic_symbol=NULL;
}

static int semantic_analyze_node(const ASTNode* node)
{
    if(node==NULL)
    {
        return 1;

    }
    if(node->type==AST_PROGRAM)
    {
        if(!semantic_analyze_node(node->left))
        {
            return 0;
        }
        if(!semantic_analyze_node(node->right))
        {
            return 0;
        }
        return 1;
    }
    if(node->type==AST_DECLARATION)
    {
        if(semantic_symbol_exists(node->value))
        {
            fprintf(stderr, 
                "Semantic error: Variable '%s' is already declared!\n", node->value);
             return 0;   
        }
        semantic_add_symbol(node->value);
        return 1;
    }

    if(node->type==AST_PRINT)
    {
        return semantic_analyze_node(node->left);
    }

    if(node->type==AST_ASSIGNMENT)
    {
        if(!semantic_symbol_exists(node->value))
        {
            fprintf(stderr,
                 "Semantic error: Variable '%s' is not declared!", node->value);
            return 0;     
        }
        return semantic_analyze_node(node->right);
    }

    if(node->type==AST_IDENTIFIER)
    {
        if(!semantic_symbol_exists(node->value))
        {
            fprintf(stderr,
                 "Semantic error: Variable '%s' is not declared!\n", node->value); 
            return 0;   
        }
        return 1;
    }

    if(node->type==AST_BINARY)
    {
        if(!semantic_analyze_node(node->left))
        {
            return 0;
        }
        return semantic_analyze_node(node->right);
    }
       
    return 1;
}   

int ast_semantic_analysis(const ASTNode* node)
{
    semantic_clear_symbol();
    return semantic_analyze_node(node);
}
