
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

typedef struct Scope
{
    Variable *variables;
    struct Scope *parent;
} Scope;

typedef struct SemanticSymbol
{
    char *name;
    DataType type;
    int line_number;
    struct SemanticSymbol *next;
} SemanticSymbol;

typedef struct SemanticScope
{
    SemanticSymbol *symbols;
    struct SemanticScope *parent;
} SemanticScope;

static Scope *current_scope = NULL;
static SemanticScope *semantic_scope = NULL;

static Variable *find_variable(const char *name);
static void scope_push(void);
static void scope_pop(void);
static void set_variable_in_current_scope(
    const char *name, DataType type, const char *value);

static SemanticSymbol *semantic_find_symbol(const char *name);
static SemanticSymbol *semantic_find_current_symbol(const char *name);
static void semantic_scope_push(void);
static void semantic_scope_pop(void);
static void semantic_add_symbol(
    const char *name, DataType type, int line_number);
static void semantic_clear_symbols(void);

static DataType semantic_get_expression_type(const ASTNode *node);
static int semantic_analyze_node(const ASTNode *node);
static int ast_is_boolean_expression(const ASTNode *node);
static int ast_evaluate_float_expression(const ASTNode *node);

ASTNode *ast_create(ASTNodeType type, const char *value,
                    ASTNode *left, ASTNode *right, int line_number)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }

    node->type = type;
    node->value = value != NULL ? strdup(value) : NULL;
    node->data_type = NULL;
    node->line_number = line_number;
    node->left = left;
    node->right = right;
    node->third = NULL;

    if (value != NULL && node->value == NULL)
    {
        free(node);
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }

    return node;
}

void ast_print(const ASTNode *node, int depth)
{
    if (node == NULL)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    switch (node->type)
    {
    case AST_PROGRAM:
        printf("PROGRAM (line: %d)\n", node->line_number);
        break;
    case AST_INT:
        printf("INTEGER: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_FLOAT:
        printf("FLOAT: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_IDENTIFIER:
        printf("IDENTIFIER: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_BINARY:
        printf("BINARY: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_UNARY:
        printf("UNARY: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_BOOL:
        printf("BOOLEAN: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_PRINT:
        printf("PRINT (line: %d)\n", node->line_number);
        break;
    case AST_DECLARATION:
        printf("DECLARATION: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_CHAR:
        printf("CHARACTER: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_STRING:
        printf("STRING: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_ASSIGNMENT:
        printf("ASSIGNMENT: %s (line: %d)\n", node->value, node->line_number);
        break;
    case AST_IF:
        printf("IF (line: %d)\n", node->line_number);
        break;
    case AST_DO_WHILE:
        printf("DO-WHILE (line: %d)\n", node->line_number);
        break;
    default:
        printf("UNKNOWN NODE (line: %d)\n", node->line_number);
        break;
    }

    ast_print(node->left, depth + 1);
    ast_print(node->right, depth + 1);
    ast_print(node->third, depth + 1);
}

void ast_free(ASTNode *node)
{
    if (node == NULL)
        return;

    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->third);

    free(node->data_type);
    free(node->value);
    free(node);
}

static void scope_push(void)
{
    Scope *new_scope = malloc(sizeof(Scope));

    if (new_scope == NULL)
    {
        printf("Error: Memory allocation failed!\n");
        exit(1);
    }

    new_scope->variables = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

static void scope_pop(void)
{
    if (current_scope == NULL)
        return;

    Variable *current = current_scope->variables;

    while (current != NULL)
    {
        Variable *next = current->next;

        if (current->type == TYPE_STRING)
            free(current->value.string_value);

        free(current->name);
        free(current);
        current = next;
    }

    Scope *parent = current_scope->parent;

    free(current_scope);
    current_scope = parent;
}

static Variable *find_variable(const char *name)
{
    Scope *scope = current_scope;

    while (scope != NULL)
    {
        Variable *current = scope->variables;

        while (current != NULL)
        {
            if (strcmp(current->name, name) == 0)
                return current;

            current = current->next;
        }

        scope = scope->parent;
    }

    return NULL;
}

static void set_variable_in_current_scope(
    const char *name, DataType type, const char *value)
{
    if (name == NULL || value == NULL)
    {
        printf("Error: Invalid variable value!\n");
        exit(1);
    }

    if (current_scope == NULL)
        scope_push();

    Variable *current = current_scope->variables;

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            break;

        current = current->next;
    }

    if (current == NULL)
    {
        current = malloc(sizeof(Variable));

        if (current == NULL)
        {
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }

        current->name = strdup(name);

        if (current->name == NULL)
        {
            free(current);
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }

        current->next = current_scope->variables;
        current_scope->variables = current;
    }
    else if (current->type == TYPE_STRING)
    {
        free(current->value.string_value);
        current->value.string_value = NULL;
    }

    current->type = type;

    switch (type)
    {
    case TYPE_INT:
        current->value.int_value = atoi(value);
        break;

    case TYPE_FLOAT:
        current->value.float_value = (float)atof(value);
        break;

    case TYPE_BOOL:
        current->value.bool_value = strcmp(value, "true") == 0;
        break;

    case TYPE_CHAR:
        current->value.char_value = value[0];
        break;

    case TYPE_STRING:
        current->value.string_value = strdup(value);

        if (current->value.string_value == NULL)
        {
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }
        break;
    }
}

void ast_set_variable(const char *name, DataType type, const char *value)
{
    if (name == NULL || value == NULL)
    {
        printf("Error: Invalid variable value!\n");
        exit(1);
    }

    if (current_scope == NULL)
        scope_push();

    Variable *variable = find_variable(name);

    if (variable == NULL)
    {
        set_variable_in_current_scope(name, type, value);
        return;
    }

    if (variable->type == TYPE_STRING)
    {
        free(variable->value.string_value);
        variable->value.string_value = NULL;
    }

    variable->type = type;

    switch (type)
    {
    case TYPE_INT:
        variable->value.int_value = atoi(value);
        break;
    case TYPE_FLOAT:
        variable->value.float_value = (float)atof(value);
        break;
    case TYPE_BOOL:
        variable->value.bool_value = strcmp(value, "true") == 0;
        break;
    case TYPE_CHAR:
        variable->value.char_value = value[0];
        break;
    case TYPE_STRING:
        variable->value.string_value = strdup(value);

        if (variable->value.string_value == NULL)
        {
            printf("Error: Memory allocation failed!\n");
            exit(1);
        }
        break;
    }
}

Variable *ast_get_variable(const char *name)
{
    Variable *variable = find_variable(name);

    if (variable == NULL)
    {
        printf("Error: Undefined Variable: %s\n", name);
        exit(1);
    }

    return variable;
}

int ast_evaluate(const ASTNode *node)
{
    if (node == NULL)
    {
        printf("Error: Cannot evaluate an empty AST node.\n");
        exit(1);
    }

    switch (node->type)
    {
    case AST_INT:
        return atoi(node->value);

    case AST_BOOL:
        return strcmp(node->value, "true") == 0;

    case AST_IDENTIFIER:
    {
        Variable *variable = ast_get_variable(node->value);

        if (variable->type == TYPE_INT)
            return variable->value.int_value;

        if (variable->type == TYPE_BOOL)
            return variable->value.bool_value;

        printf("Error: Expected an integer or boolean variable: %s\n",
               node->value);
        exit(1);
    }

    case AST_ASSIGNMENT:
    {
        int result = ast_evaluate(node->right);
        Variable *variable = ast_get_variable(node->value);

        if (variable->type != TYPE_INT)
        {
            printf("Error: Integer assignment requires an integer variable: %s\n",
                   node->value);
            exit(1);
        }

        char text[32];
        snprintf(text, sizeof(text), "%d", result);
        ast_set_variable(node->value, TYPE_INT, text);

        return result;
    }

    case AST_UNARY:
        if (strcmp(node->value, "!") == 0)
            return !ast_evaluate(node->left);

        printf("Error: Unsupported unary operator: %s\n", node->value);
        exit(1);

    case AST_BINARY:
    {
        int left = ast_evaluate(node->left);

        if (strcmp(node->value, "&&") == 0 && !left)
            return 0;

        if (strcmp(node->value, "||") == 0 && left)
            return 1;

        int right = ast_evaluate(node->right);

        if (strcmp(node->value, "+") == 0)
            return left + right;
        if (strcmp(node->value, "-") == 0)
            return left - right;
        if (strcmp(node->value, "*") == 0)
            return left * right;

        if (strcmp(node->value, "/") == 0)
        {
            if (right == 0)
            {
                printf("Error: Division by zero!\n");
                exit(1);
            }
            return left / right;
        }

        if (strcmp(node->value, "%") == 0)
        {
            if (right == 0)
            {
                printf("Error: Modulo by zero!\n");
                exit(1);
            }
            return left % right;
        }

        if (strcmp(node->value, "<") == 0)
            return left < right;
        if (strcmp(node->value, ">") == 0)
            return left > right;
        if (strcmp(node->value, "<=") == 0)
            return left <= right;
        if (strcmp(node->value, ">=") == 0)
            return left >= right;
        if (strcmp(node->value, "==") == 0)
            return left == right;
        if (strcmp(node->value, "!=") == 0)
            return left != right;
        if (strcmp(node->value, "&&") == 0)
            return left && right;
        if (strcmp(node->value, "||") == 0)
            return left || right;

        printf("Error: Unsupported operator: %s\n", node->value);
        exit(1);
    }

    default:
        printf("Error: Unsupported expression type!\n");
        exit(1);
    }
}

float ast_evaluate_float(const ASTNode *node)
{
    if (node == NULL)
    {
        printf("Error: Cannot evaluate an empty expression.\n");
        exit(1);
    }

    switch (node->type)
    {
    case AST_FLOAT:
        return (float)atof(node->value);

    case AST_INT:
        return (float)atoi(node->value);

    case AST_BOOL:
        return strcmp(node->value, "true") == 0 ? 1.0f : 0.0f;

    case AST_IDENTIFIER:
    {
        Variable *variable = ast_get_variable(node->value);

        if (variable->type == TYPE_FLOAT)
            return variable->value.float_value;
        if (variable->type == TYPE_INT)
            return (float)variable->value.int_value;
        if (variable->type == TYPE_BOOL)
            return variable->value.bool_value ? 1.0f : 0.0f;

        printf("Error: Variable %s is not numeric or boolean.\n",
               node->value);
        exit(1);
    }

    case AST_ASSIGNMENT:
    {
        float result = ast_evaluate_float(node->right);
        Variable *variable = ast_get_variable(node->value);

        if (variable->type != TYPE_FLOAT)
        {
            printf("Error: Float assignment requires a float variable: %s\n",
                   node->value);
            exit(1);
        }

        char text[32];
        snprintf(text, sizeof(text), "%.9g", (double)result);
        ast_set_variable(node->value, TYPE_FLOAT, text);

        return result;
    }

    case AST_UNARY:
        if (strcmp(node->value, "!") == 0)
            return !ast_evaluate_float(node->left);

        printf("Error: Unsupported unary operator: %s\n", node->value);
        exit(1);

    case AST_BINARY:
    {
        float left = ast_evaluate_float(node->left);

        if (strcmp(node->value, "&&") == 0 && !left)
            return 0.0f;

        if (strcmp(node->value, "||") == 0 && left)
            return 1.0f;

        float right = ast_evaluate_float(node->right);

        if (strcmp(node->value, "+") == 0)
            return left + right;
        if (strcmp(node->value, "-") == 0)
            return left - right;
        if (strcmp(node->value, "*") == 0)
            return left * right;

        if (strcmp(node->value, "/") == 0)
        {
            if (right == 0.0f)
            {
                printf("Error: Division by zero!\n");
                exit(1);
            }
            return left / right;
        }

        if (strcmp(node->value, "<") == 0)
            return left < right;
        if (strcmp(node->value, ">") == 0)
            return left > right;
        if (strcmp(node->value, "<=") == 0)
            return left <= right;
        if (strcmp(node->value, ">=") == 0)
            return left >= right;
        if (strcmp(node->value, "==") == 0)
            return left == right;
        if (strcmp(node->value, "!=") == 0)
            return left != right;
        if (strcmp(node->value, "&&") == 0)
            return left && right;
        if (strcmp(node->value, "||") == 0)
            return left || right;

        printf("Error: Unsupported operator: %s\n", node->value);
        exit(1);
    }

    default:
        printf("Error: Unsupported expression type!\n");
        exit(1);
    }
}

static int ast_evaluate_float_expression(const ASTNode *node)
{
    if (node == NULL)
        return 0;

    if (node->type == AST_FLOAT)
        return 1;

    if (node->type == AST_IDENTIFIER)
    {
        Variable *variable = ast_get_variable(node->value);
        return variable->type == TYPE_FLOAT;
    }

    if (node->type == AST_BINARY)
    {
        if (strcmp(node->value, "<") == 0 ||
            strcmp(node->value, ">") == 0 ||
            strcmp(node->value, "<=") == 0 ||
            strcmp(node->value, ">=") == 0 ||
            strcmp(node->value, "==") == 0 ||
            strcmp(node->value, "!=") == 0 ||
            strcmp(node->value, "&&") == 0 ||
            strcmp(node->value, "||") == 0)
            return 0;

        return ast_evaluate_float_expression(node->left) ||
               ast_evaluate_float_expression(node->right);
    }

    if (node->type == AST_UNARY)
        return 0;

    return 0;
}

static int ast_is_boolean_expression(const ASTNode *node)
{
    if (node == NULL)
        return 0;

    if (node->type == AST_BOOL)
        return 1;

    if (node->type == AST_IDENTIFIER)
    {
        Variable *variable = ast_get_variable(node->value);
        return variable->type == TYPE_BOOL;
    }

    if (node->type == AST_UNARY)
        return strcmp(node->value, "!") == 0;

    if (node->type == AST_BINARY)
    {
        const char *op = node->value;

        return strcmp(op, "<") == 0 ||
               strcmp(op, ">") == 0 ||
               strcmp(op, "<=") == 0 ||
               strcmp(op, ">=") == 0 ||
               strcmp(op, "==") == 0 ||
               strcmp(op, "!=") == 0 ||
               strcmp(op, "&&") == 0 ||
               strcmp(op, "||") == 0;
    }

    return 0;
}

void ast_execute(const ASTNode *node)
{
    if (node == NULL)
        return;

    if (current_scope == NULL)
        scope_push();

    switch (node->type)
    {
    case AST_PROGRAM:
        ast_execute(node->left);
        ast_execute(node->right);
        break;

    case AST_DECLARATION:
    {
        if (node->left == NULL || node->value == NULL ||
            node->data_type == NULL)
        {
            printf("Error: Invalid declaration!\n");
            exit(1);
        }

        DataType type;

        if (strcmp(node->data_type, "int") == 0)
            type = TYPE_INT;
        else if (strcmp(node->data_type, "float") == 0)
            type = TYPE_FLOAT;
        else if (strcmp(node->data_type, "bool") == 0)
            type = TYPE_BOOL;
        else if (strcmp(node->data_type, "char") == 0)
            type = TYPE_CHAR;
        else if (strcmp(node->data_type, "string") == 0)
            type = TYPE_STRING;
        else
        {
            printf("Error: Unknown declaration type!\n");
            exit(1);
        }

        char text[64];

        switch (type)
        {
        case TYPE_INT:
            snprintf(text, sizeof(text), "%d",
                     ast_evaluate(node->left));
            set_variable_in_current_scope(node->value, type, text);
            break;

        case TYPE_FLOAT:
            snprintf(text, sizeof(text), "%.9g",
                     (double)ast_evaluate_float(node->left));
            set_variable_in_current_scope(node->value, type, text);
            break;

        case TYPE_BOOL:
        {
            int result;

            if (ast_evaluate_float_expression(node->left))
                result = ast_evaluate_float(node->left) != 0.0f;
            else
                result = ast_evaluate(node->left);

            set_variable_in_current_scope(
                node->value, TYPE_BOOL, result ? "true" : "false");
            break;
        }

        case TYPE_CHAR:
            if (node->left->type != AST_CHAR)
            {
                printf("Error: Character initializer must be a character literal.\n");
                exit(1);
            }
            set_variable_in_current_scope(
                node->value, TYPE_CHAR, node->left->value);
            break;

        case TYPE_STRING:
            if (node->left->type != AST_STRING)
            {
                printf("Error: String initializer must be a string literal.\n");
                exit(1);
            }
            set_variable_in_current_scope(
                node->value, TYPE_STRING, node->left->value);
            break;
        }

        break;
    }

    case AST_IF:
    {
        int condition;

        if (ast_evaluate_float_expression(node->left))
            condition = ast_evaluate_float(node->left) != 0.0f;
        else
            condition = ast_evaluate(node->left);

        if (condition)
        {
            scope_push();
            ast_execute(node->right);
            scope_pop();
        }
        else if (node->third != NULL)
        {
            scope_push();
            ast_execute(node->third);
            scope_pop();
        }

        break;
    }

    case AST_DO_WHILE:
        do
        {
            scope_push();
            ast_execute(node->left);
            scope_pop();

            int condition;

            if (ast_evaluate_float_expression(node->right))
                condition = ast_evaluate_float(node->right) != 0.0f;
            else
                condition = ast_evaluate(node->right);

            if (!condition)
                break;

        } while (1);
        break;

    case AST_ASSIGNMENT:
    {
        Variable *variable = ast_get_variable(node->value);

        if (variable->type == TYPE_INT)
            ast_evaluate(node);
        else if (variable->type == TYPE_FLOAT)
            ast_evaluate_float(node);
        else if (variable->type == TYPE_BOOL)
        {
            int result;

            if (ast_evaluate_float_expression(node->right))
                result = ast_evaluate_float(node->right) != 0.0f;
            else
                result = ast_evaluate(node->right);

            variable->value.bool_value = result != 0;
        }

        break;
    }

    case AST_PRINT:
    {
        if (node->left == NULL)
            break;

        if (node->left->type == AST_FLOAT)
            printf("%g\n", atof(node->left->value));

        else if (node->left->type == AST_BOOL)
            printf("%s\n", node->left->value);

        else if (node->left->type == AST_CHAR)
            printf("%c\n", node->left->value[0]);

        else if (node->left->type == AST_STRING)
            printf("%s\n", node->left->value);

        else if (node->left->type == AST_IDENTIFIER)
        {
            Variable *variable = ast_get_variable(node->left->value);

            switch (variable->type)
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
        else if (ast_is_boolean_expression(node->left))
        {
            int result;

            if (ast_evaluate_float_expression(node->left))
                result = ast_evaluate_float(node->left) != 0.0f;
            else
                result = ast_evaluate(node->left);

            printf("%s\n", result ? "true" : "false");
        }
        else if (ast_evaluate_float_expression(node->left))
            printf("%g\n", ast_evaluate_float(node->left));
        else
            printf("%d\n", ast_evaluate(node->left));

        break;
    }

    default:
        break;
    }
}

void ast_free_variables(void)
{
    while (current_scope != NULL)
        scope_pop();
}

void ast_export(const ASTNode *node)
{
    printf("----AST START----\n");
    ast_print(node, 0);
    printf("\n---AST END----\n");
}

static void semantic_scope_push(void)
{
    SemanticScope *new_scope = malloc(sizeof(SemanticScope));

    if (new_scope == NULL)
    {
        fprintf(stderr, "Semantic error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    new_scope->symbols = NULL;
    new_scope->parent = semantic_scope;
    semantic_scope = new_scope;
}

static void semantic_scope_pop(void)
{
    if (semantic_scope == NULL)
        return;

    SemanticSymbol *current = semantic_scope->symbols;

    while (current != NULL)
    {
        SemanticSymbol *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }

    SemanticScope *parent = semantic_scope->parent;
    free(semantic_scope);
    semantic_scope = parent;
}

static SemanticSymbol *semantic_find_current_symbol(const char *name)
{
    if (semantic_scope == NULL)
        return NULL;

    SemanticSymbol *current = semantic_scope->symbols;

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

static SemanticSymbol *semantic_find_symbol(const char *name)
{
    SemanticScope *scope = semantic_scope;

    while (scope != NULL)
    {
        SemanticSymbol *current = scope->symbols;

        while (current != NULL)
        {
            if (strcmp(current->name, name) == 0)
                return current;

            current = current->next;
        }

        scope = scope->parent;
    }

    return NULL;
}

static void semantic_add_symbol(
    const char *name, DataType type, int line_number)
{
    SemanticSymbol *symbol = malloc(sizeof(SemanticSymbol));

    if (symbol == NULL)
    {
        fprintf(stderr, "Semantic error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    symbol->name = strdup(name);

    if (symbol->name == NULL)
    {
        free(symbol);
        fprintf(stderr, "Semantic error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    symbol->type = type;
    symbol->line_number = line_number;
    symbol->next = semantic_scope->symbols;
    semantic_scope->symbols = symbol;
}

static void semantic_clear_symbols(void)
{
    while (semantic_scope != NULL)
        semantic_scope_pop();
}

static DataType semantic_get_expression_type(const ASTNode *node)
{
    if (node == NULL)
        return (DataType)-1;

    switch (node->type)
    {
    case AST_INT:
        return TYPE_INT;
    case AST_FLOAT:
        return TYPE_FLOAT;
    case AST_BOOL:
        return TYPE_BOOL;
    case AST_CHAR:
        return TYPE_CHAR;
    case AST_STRING:
        return TYPE_STRING;

    case AST_IDENTIFIER:
    {
        SemanticSymbol *symbol = semantic_find_symbol(node->value);

        if (symbol == NULL)
            return (DataType)-1;

        return symbol->type;
    }

    case AST_UNARY:
    {
        DataType operand_type =
            semantic_get_expression_type(node->left);

        if (strcmp(node->value, "!") == 0 &&
            operand_type == TYPE_BOOL)
            return TYPE_BOOL;

        return (DataType)-1;
    }

    case AST_BINARY:
    {
        DataType left_type =
            semantic_get_expression_type(node->left);
        DataType right_type =
            semantic_get_expression_type(node->right);

        if (left_type == (DataType)-1 ||
            right_type == (DataType)-1)
            return (DataType)-1;

        if (strcmp(node->value, "+") == 0 ||
            strcmp(node->value, "-") == 0 ||
            strcmp(node->value, "*") == 0 ||
            strcmp(node->value, "/") == 0 ||
            strcmp(node->value, "%") == 0)
        {
            if ((left_type != TYPE_INT && left_type != TYPE_FLOAT) ||
                (right_type != TYPE_INT && right_type != TYPE_FLOAT))
                return (DataType)-1;

            if (strcmp(node->value, "%") == 0)
            {
                if (left_type != TYPE_INT || right_type != TYPE_INT)
                    return (DataType)-1;

                return TYPE_INT;
            }

            if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT)
                return TYPE_FLOAT;

            return TYPE_INT;
        }

        if (strcmp(node->value, "<") == 0 ||
            strcmp(node->value, ">") == 0 ||
            strcmp(node->value, "<=") == 0 ||
            strcmp(node->value, ">=") == 0 ||
            strcmp(node->value, "==") == 0 ||
            strcmp(node->value, "!=") == 0)
        {
            if ((left_type != TYPE_INT && left_type != TYPE_FLOAT) ||
                (right_type != TYPE_INT && right_type != TYPE_FLOAT))
                return (DataType)-1;

            return TYPE_BOOL;
        }

        if (strcmp(node->value, "&&") == 0 ||
            strcmp(node->value, "||") == 0)
        {
            if (left_type != TYPE_BOOL || right_type != TYPE_BOOL)
                return (DataType)-1;

            return TYPE_BOOL;
        }

        return (DataType)-1;
    }

    default:
        return (DataType)-1;
    }
}

static int semantic_analyze_node(const ASTNode *node)
{
    if (node == NULL)
        return 1;

    if (node->type == AST_PROGRAM)
    {
        if (!semantic_analyze_node(node->left))
            return 0;

        return semantic_analyze_node(node->right);
    }

    if (node->type == AST_DECLARATION)
    {
        if (node->left == NULL || node->value == NULL ||
            node->data_type == NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Invalid declaration!\n",
                    node->line_number);
            return 0;
        }

        if (semantic_find_current_symbol(node->value) != NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is already declared in this scope!\n",
                    node->line_number, node->value);
            return 0;
        }

        DataType declared_type;

        if (strcmp(node->data_type, "int") == 0)
            declared_type = TYPE_INT;
        else if (strcmp(node->data_type, "float") == 0)
            declared_type = TYPE_FLOAT;
        else if (strcmp(node->data_type, "bool") == 0)
            declared_type = TYPE_BOOL;
        else if (strcmp(node->data_type, "char") == 0)
            declared_type = TYPE_CHAR;
        else if (strcmp(node->data_type, "string") == 0)
            declared_type = TYPE_STRING;
        else
        {
            fprintf(stderr,
                    "Semantic error at line %d: Unknown declaration type!\n",
                    node->line_number);
            return 0;
        }

        if (!semantic_analyze_node(node->left))
            return 0;

        DataType initializer_type =
            semantic_get_expression_type(node->left);

        if (initializer_type != declared_type)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Initializer type does not match declared type for '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        semantic_add_symbol(node->value, declared_type, node->line_number);
        return 1;
    }

    if (node->type == AST_PRINT)
        return semantic_analyze_node(node->left);

    if (node->type == AST_ASSIGNMENT)
    {
        SemanticSymbol *symbol = semantic_find_symbol(node->value);

        if (symbol == NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is not declared!\n",
                    node->line_number, node->value);
            return 0;
        }

        if (!semantic_analyze_node(node->right))
            return 0;

        DataType expression_type =
            semantic_get_expression_type(node->right);

        if (expression_type == (DataType)-1)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Invalid assignment expression.\n",
                    node->line_number);
            return 0;
        }

        if (symbol->type != expression_type)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Cannot assign incompatible type to '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_IDENTIFIER)
    {
        if (semantic_find_symbol(node->value) == NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is not declared!\n",
                    node->line_number, node->value);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_UNARY)
    {
        if (!semantic_analyze_node(node->left))
            return 0;

        if (semantic_get_expression_type(node) == (DataType)-1)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Invalid operand for '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_BINARY)
    {
        if (!semantic_analyze_node(node->left) ||
            !semantic_analyze_node(node->right))
            return 0;

        if (semantic_get_expression_type(node) == (DataType)-1)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Invalid operands for '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_IF)
    {
        if (!semantic_analyze_node(node->left))
            return 0;

        if (semantic_get_expression_type(node->left) != TYPE_BOOL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: If condition must be boolean.\n",
                    node->line_number);
            return 0;
        }

        semantic_scope_push();
        int result = semantic_analyze_node(node->right);
        semantic_scope_pop();

        if (!result)
            return 0;

        if (node->third != NULL)
        {
            semantic_scope_push();
            result = semantic_analyze_node(node->third);
            semantic_scope_pop();

            if (!result)
                return 0;
        }

        return 1;
    }

    if (node->type == AST_DO_WHILE)
    {
        semantic_scope_push();
        int result = semantic_analyze_node(node->left);
        semantic_scope_pop();

        if (!result)
            return 0;

        if (!semantic_analyze_node(node->right))
            return 0;

        if (semantic_get_expression_type(node->right) != TYPE_BOOL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Do-while condition must be boolean.\n",
                    node->line_number);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_STRING || node->type == AST_CHAR ||
        node->type == AST_INT || node->type == AST_FLOAT ||
        node->type == AST_BOOL)
        return 1;

    return 1;
}

int ast_semantic_analysis(const ASTNode *node)
{
    semantic_clear_symbols();
    semantic_scope_push();

    int result = semantic_analyze_node(node);

    semantic_clear_symbols();

    return result;
}
