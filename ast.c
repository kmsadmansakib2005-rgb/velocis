
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ast.h"

typedef struct Scope
{
    Variable *variables;
    struct Scope *parent;
    int level;
} Scope;

typedef struct SemanticSymbol
{
    char *name;
    DataType type;
    int line_number;
    int initialized;
    struct SemanticSymbol *next;
} SemanticSymbol;

typedef struct SemanticScope
{
    SemanticSymbol *symbols;
    struct SemanticScope *parent;
} SemanticScope;

static Scope *current_scope = NULL;
static SemanticScope *semantic_scope = NULL;

static void scope_push(void);
static void scope_pop(void);
static Variable *find_variable(const char *name);

static void semantic_scope_push(void);
static void semantic_scope_pop(void);
static void semantic_clear_symbols(void);
static SemanticSymbol *semantic_find_symbol(const char *name);
static SemanticSymbol *semantic_find_current_symbol(const char *name);
static void semantic_add_symbol(const char *name, DataType type,
                                int line, int initialized);

static int semantic_analyze_node(const ASTNode *node);
static DataType semantic_get_expression_type(const ASTNode *node,
                                             DataType expected_type);

static int ast_evaluate_float_expression(const ASTNode *node);
static int ast_is_boolean_expression(const ASTNode *node);
static void ast_read_input(DataType type, char *text, size_t size);
static float ast_evaluate_input(DataType type);


ASTNode *ast_create(ASTNodeType type, const char *value,
                    ASTNode *left, ASTNode *right, int line_number)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->data_type = NULL;
    node->line_number = line_number;
    node->left = left;
    node->right = right;
    node->third = NULL;

    if (value != NULL && node->value == NULL)
    {
        free(node);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
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
    case AST_INPUT:
        printf("INPUT (line: %d)\n", node->line_number);
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

    free(node->value);
    free(node->data_type);
    free(node);
}

void ast_export(const ASTNode *node)
{
    printf("----AST START----\n");
    ast_print(node, 0);
    printf("\n---AST END----\n");
}

static void scope_push(void)
{
    Scope *new_scope = malloc(sizeof(Scope));

    if (new_scope == NULL)
    {
        fprintf(stderr, "Runtime error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    new_scope->variables = NULL;
    new_scope->parent = current_scope;
    new_scope->level = current_scope ? current_scope->level + 1 : 0;

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
    for (Scope *scope = current_scope; scope != NULL; scope = scope->parent)
    {
        for (Variable *variable = scope->variables;
             variable != NULL; variable = variable->next)
        {
            if (strcmp(variable->name, name) == 0)
                return variable;
        }
    }

    return NULL;
}

static Variable *find_current_variable(const char *name)
{
    if (current_scope == NULL)
        return NULL;

    for (Variable *variable = current_scope->variables;
         variable != NULL; variable = variable->next)
    {
        if (strcmp(variable->name, name) == 0)
            return variable;
    }

    return NULL;
}

static Variable *create_variable(const char *name, DataType type,
                                 int line, int initialized)
{
    if (current_scope == NULL)
        scope_push();

    if (find_current_variable(name) != NULL)
    {
        fprintf(stderr, "Runtime error: Duplicate variable '%s'.\n", name);
        exit(EXIT_FAILURE);
    }

    Variable *variable = calloc(1, sizeof(Variable));

    if (variable == NULL)
    {
        fprintf(stderr, "Runtime error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    variable->name = strdup(name);

    if (variable->name == NULL)
    {
        free(variable);
        fprintf(stderr, "Runtime error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    variable->type = type;
    variable->scope_level = current_scope->level;
    variable->declaration_line = line;
    variable->initialized = initialized;

    variable->next = current_scope->variables;
    current_scope->variables = variable;

    return variable;
}

static void set_variable_value(Variable *variable, const char *value)
{
    if (variable == NULL || value == NULL)
        return;

    if (variable->type == TYPE_STRING)
    {
        free(variable->value.string_value);
        variable->value.string_value = NULL;
    }

    switch (variable->type)
    {
    case TYPE_INT:
        variable->value.int_value = (int)strtol(value, NULL, 10);
        break;

    case TYPE_FLOAT:
        variable->value.float_value = strtof(value, NULL);
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
            fprintf(stderr, "Runtime error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        break;
    }

    variable->initialized = 1;
}

void ast_set_variable(const char *name, DataType type, const char *value)
{
    if (name == NULL || value == NULL)
    {
        fprintf(stderr, "Runtime error: Invalid variable value.\n");
        exit(EXIT_FAILURE);
    }

    if (current_scope == NULL)
        scope_push();

    Variable *variable = find_variable(name);

    if (variable == NULL)
        variable = create_variable(name, type, 0, 0);

    if (variable->type != type)
    {
        fprintf(stderr, "Runtime error: Type mismatch for '%s'.\n", name);
        exit(EXIT_FAILURE);
    }

    set_variable_value(variable, value);
}

Variable *ast_get_variable(const char *name)
{
    Variable *variable = find_variable(name);

    if (variable == NULL)
    {
        fprintf(stderr, "Runtime error: Undefined variable '%s'.\n", name);
        exit(EXIT_FAILURE);
    }

    if (!variable->initialized)
    {
        fprintf(stderr, "Runtime error: Variable '%s' is not initialized.\n", name);
        exit(EXIT_FAILURE);
    }

    return variable;
}

void ast_free_variables(void)
{
    while (current_scope != NULL)
        scope_pop();
}

static void ast_read_input(DataType type, char *text, size_t size)
{
    printf("Enter value: ");
    fflush(stdout);

    if (fgets(text, (int)size, stdin) == NULL)
    {
        fprintf(stderr, "Runtime error: Failed to read input.\n");
        exit(EXIT_FAILURE);
    }

    text[strcspn(text, "\r\n")] = '\0';

    char *end = NULL;
    errno = 0;

    switch (type)
    {
    case TYPE_INT:
        strtol(text, &end, 10);

        if (end == text || *end != '\0' || errno == ERANGE)
        {
            fprintf(stderr, "Runtime error: Invalid integer input.\n");
            exit(EXIT_FAILURE);
        }
        break;

    case TYPE_FLOAT:
        strtof(text, &end);

        if (end == text || *end != '\0' || errno == ERANGE)
        {
            fprintf(stderr, "Runtime error: Invalid float input.\n");
            exit(EXIT_FAILURE);
        }
        break;

    case TYPE_BOOL:
        if (strcmp(text, "true") != 0 && strcmp(text, "false") != 0)
        {
            fprintf(stderr, "Runtime error: Enter true or false.\n");
            exit(EXIT_FAILURE);
        }
        break;

    case TYPE_CHAR:
        if (strlen(text) != 1)
        {
            fprintf(stderr, "Runtime error: Enter exactly one character.\n");
            exit(EXIT_FAILURE);
        }
        break;

    case TYPE_STRING:
        break;
    }
}

static float ast_evaluate_input(DataType type)
{
    char text[256];

    ast_read_input(type, text, sizeof(text));

    if (type == TYPE_INT)
        return (float)strtol(text, NULL, 10);

    if (type == TYPE_FLOAT)
        return strtof(text, NULL);

    if (type == TYPE_BOOL)
        return strcmp(text, "true") == 0 ? 1.0f : 0.0f;

    fprintf(stderr, "Runtime error: Unsupported numeric input type.\n");
    exit(EXIT_FAILURE);
}


int ast_evaluate(const ASTNode *node)
{
    if (node == NULL)
    {
        fprintf(stderr, "Runtime error: Empty expression.\n");
        exit(EXIT_FAILURE);
    }

    switch (node->type)
    {
    case AST_INT:
        return (int)strtol(node->value, NULL, 10);

    case AST_BOOL:
        return strcmp(node->value, "true") == 0;

    case AST_IDENTIFIER:
    {
        Variable *variable = ast_get_variable(node->value);

        if (variable->type == TYPE_INT)
            return variable->value.int_value;

        if (variable->type == TYPE_BOOL)
            return variable->value.bool_value;

        fprintf(stderr, "Runtime error: '%s' is not an integer or boolean.\n",
                node->value);
        exit(EXIT_FAILURE);
    }

    case AST_ASSIGNMENT:
    {
        Variable *variable = find_variable(node->value);

        if (variable == NULL)
        {
            fprintf(stderr, "Runtime error: Undefined variable '%s'.\n",
                    node->value);
            exit(EXIT_FAILURE);
        }

        int result = ast_evaluate(node->right);

        if (variable->type != TYPE_INT)
        {
            fprintf(stderr, "Runtime error: Integer assignment type mismatch.\n");
            exit(EXIT_FAILURE);
        }

        char text[64];
        snprintf(text, sizeof(text), "%d", result);
        set_variable_value(variable, text);

        return result;
    }

    case AST_UNARY:
        if (strcmp(node->value, "!") == 0)
            return !ast_evaluate(node->left);

        fprintf(stderr, "Runtime error: Unsupported unary operator.\n");
        exit(EXIT_FAILURE);

    case AST_BINARY:
    {
        int left = ast_evaluate(node->left);

        if (strcmp(node->value, "&&") == 0 && !left)
            return 0;

        if (strcmp(node->value, "||") == 0 && left)
            return 1;

        int right = ast_evaluate(node->right);

        if (strcmp(node->value, "+") == 0) return left + right;
        if (strcmp(node->value, "-") == 0) return left - right;
        if (strcmp(node->value, "*") == 0) return left * right;

        if (strcmp(node->value, "/") == 0)
        {
            if (right == 0)
            {
                fprintf(stderr, "Runtime error: Division by zero.\n");
                exit(EXIT_FAILURE);
            }
            return left / right;
        }

        if (strcmp(node->value, "%") == 0)
        {
            if (right == 0)
            {
                fprintf(stderr, "Runtime error: Modulo by zero.\n");
                exit(EXIT_FAILURE);
            }
            return left % right;
        }

        if (strcmp(node->value, "<") == 0) return left < right;
        if (strcmp(node->value, ">") == 0) return left > right;
        if (strcmp(node->value, "<=") == 0) return left <= right;
        if (strcmp(node->value, ">=") == 0) return left >= right;
        if (strcmp(node->value, "==") == 0) return left == right;
        if (strcmp(node->value, "!=") == 0) return left != right;
        if (strcmp(node->value, "&&") == 0) return left && right;
        if (strcmp(node->value, "||") == 0) return left || right;

        fprintf(stderr, "Runtime error: Unsupported operator '%s'.\n",
                node->value);
        exit(EXIT_FAILURE);
    }

    case AST_INPUT:
        return (int)ast_evaluate_input(TYPE_INT);

    default:
        fprintf(stderr, "Runtime error: Unsupported expression type.\n");
        exit(EXIT_FAILURE);
    }
}

float ast_evaluate_float(const ASTNode *node)
{
    if (node == NULL)
    {
        fprintf(stderr, "Runtime error: Empty expression.\n");
        exit(EXIT_FAILURE);
    }

    switch (node->type)
    {
    case AST_FLOAT:
        return strtof(node->value, NULL);

    case AST_INT:
        return (float)strtol(node->value, NULL, 10);

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
            return (float)variable->value.bool_value;

        fprintf(stderr, "Runtime error: '%s' is not numeric.\n", node->value);
        exit(EXIT_FAILURE);
    }

    case AST_ASSIGNMENT:
    {
        Variable *variable = find_variable(node->value);

        if (variable == NULL || variable->type != TYPE_FLOAT)
        {
            fprintf(stderr, "Runtime error: Float assignment type mismatch.\n");
            exit(EXIT_FAILURE);
        }

        float result = ast_evaluate_float(node->right);
        char text[64];

        snprintf(text, sizeof(text), "%.9g", (double)result);
        set_variable_value(variable, text);

        return result;
    }

    case AST_UNARY:
        if (strcmp(node->value, "!") == 0)
            return !ast_evaluate_float(node->left);

        fprintf(stderr, "Runtime error: Unsupported unary operator.\n");
        exit(EXIT_FAILURE);

    case AST_BINARY:
    {
        float left = ast_evaluate_float(node->left);

        if (strcmp(node->value, "&&") == 0 && !left)
            return 0.0f;

        if (strcmp(node->value, "||") == 0 && left)
            return 1.0f;

        float right = ast_evaluate_float(node->right);

        if (strcmp(node->value, "+") == 0) return left + right;
        if (strcmp(node->value, "-") == 0) return left - right;
        if (strcmp(node->value, "*") == 0) return left * right;

        if (strcmp(node->value, "/") == 0)
        {
            if (right == 0.0f)
            {
                fprintf(stderr, "Runtime error: Division by zero.\n");
                exit(EXIT_FAILURE);
            }
            return left / right;
        }

        if (strcmp(node->value, "<") == 0) return left < right;
        if (strcmp(node->value, ">") == 0) return left > right;
        if (strcmp(node->value, "<=") == 0) return left <= right;
        if (strcmp(node->value, ">=") == 0) return left >= right;
        if (strcmp(node->value, "==") == 0) return left == right;
        if (strcmp(node->value, "!=") == 0) return left != right;
        if (strcmp(node->value, "&&") == 0) return left && right;
        if (strcmp(node->value, "||") == 0) return left || right;

        fprintf(stderr, "Runtime error: Unsupported operator '%s'.\n",
                node->value);
        exit(EXIT_FAILURE);
    }

    case AST_INPUT:
        return ast_evaluate_input(TYPE_FLOAT);

    default:
        fprintf(stderr, "Runtime error: Unsupported expression type.\n");
        exit(EXIT_FAILURE);
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
        return ast_evaluate_float_expression(node->left) ||
               ast_evaluate_float_expression(node->right);

    if (node->type == AST_UNARY)
        return ast_evaluate_float_expression(node->left);

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

/* =========================================================
   RUNTIME EXECUTION
   ========================================================= */

static DataType get_declared_type(const char *type)
{
    if (strcmp(type, "int") == 0) return TYPE_INT;
    if (strcmp(type, "float") == 0) return TYPE_FLOAT;
    if (strcmp(type, "bool") == 0) return TYPE_BOOL;
    if (strcmp(type, "char") == 0) return TYPE_CHAR;
    if (strcmp(type, "string") == 0) return TYPE_STRING;

    fprintf(stderr, "Runtime error: Unknown declaration type.\n");
    exit(EXIT_FAILURE);
}

static void evaluate_assignment(const ASTNode *node)
{
    Variable *variable = find_variable(node->value);

    if (variable == NULL)
    {
        fprintf(stderr, "Runtime error: Undefined variable '%s'.\n",
                node->value);
        exit(EXIT_FAILURE);
    }

    char text[256];

    switch (variable->type)
    {
    case TYPE_INT:
        snprintf(text, sizeof(text), "%d", ast_evaluate(node->right));
        break;

    case TYPE_FLOAT:
        snprintf(text, sizeof(text), "%.9g",
                 (double)ast_evaluate_float(node->right));
        break;

    case TYPE_BOOL:
    {
        int result;

        if (ast_evaluate_float_expression(node->right))
            result = ast_evaluate_float(node->right) != 0.0f;
        else
            result = ast_evaluate(node->right);

        snprintf(text, sizeof(text), "%s", result ? "true" : "false");
        break;
    }

    case TYPE_CHAR:
        if (node->right->type != AST_CHAR)
        {
            fprintf(stderr, "Runtime error: Character assignment requires a character literal.\n");
            exit(EXIT_FAILURE);
        }
        snprintf(text, sizeof(text), "%s", node->right->value);
        break;

    case TYPE_STRING:
        if (node->right->type != AST_STRING)
        {
            fprintf(stderr, "Runtime error: String assignment requires a string literal.\n");
            exit(EXIT_FAILURE);
        }
        snprintf(text, sizeof(text), "%s", node->right->value);
        break;
    }

    set_variable_value(variable, text);
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
        DataType type = get_declared_type(node->data_type);
        Variable *variable = create_variable(
            node->value, type, node->line_number, 0
        );

        if (node->left == NULL)
            break;

        char text[256];

        switch (type)
        {
        case TYPE_INT:
            if (node->left->type == AST_INPUT)
                ast_read_input(TYPE_INT, text, sizeof(text));
            else
                snprintf(text, sizeof(text), "%d", ast_evaluate(node->left));
            break;

        case TYPE_FLOAT:
            if (node->left->type == AST_INPUT)
                ast_read_input(TYPE_FLOAT, text, sizeof(text));
            else
                snprintf(text, sizeof(text), "%.9g",
                         (double)ast_evaluate_float(node->left));
            break;

        case TYPE_BOOL:
        {
            if (node->left->type == AST_INPUT)
            {
                ast_read_input(TYPE_BOOL, text, sizeof(text));
                break;
            }

            int result;

            if (ast_evaluate_float_expression(node->left))
                result = ast_evaluate_float(node->left) != 0.0f;
            else
                result = ast_evaluate(node->left);

            snprintf(text, sizeof(text), "%s", result ? "true" : "false");
            break;
        }

        case TYPE_CHAR:
            if (node->left->type == AST_INPUT)
                ast_read_input(TYPE_CHAR, text, sizeof(text));
            else
                snprintf(text, sizeof(text), "%s", node->left->value);
            break;

        case TYPE_STRING:
            if (node->left->type == AST_INPUT)
                ast_read_input(TYPE_STRING, text, sizeof(text));
            else
                snprintf(text, sizeof(text), "%s", node->left->value);
            break;
        }

        set_variable_value(variable, text);
        break;
    }

    case AST_ASSIGNMENT:
        evaluate_assignment(node);
        break;

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

    case AST_PRINT:
    {
        const ASTNode *expression = node->left;

        if (expression == NULL)
            break;

        if (expression->type == AST_FLOAT)
            printf("%g\n", strtof(expression->value, NULL));

        else if (expression->type == AST_BOOL)
            printf("%s\n", expression->value);

        else if (expression->type == AST_CHAR)
            printf("%c\n", expression->value[0]);

        else if (expression->type == AST_STRING)
            printf("%s\n", expression->value);

        else if (expression->type == AST_IDENTIFIER)
        {
            Variable *variable = ast_get_variable(expression->value);

            switch (variable->type)
            {
            case TYPE_INT:
                printf("%d\n", variable->value.int_value);
                break;
            case TYPE_FLOAT:
                printf("%g\n", variable->value.float_value);
                break;
            case TYPE_BOOL:
                printf("%s\n", variable->value.bool_value ? "true" : "false");
                break;
            case TYPE_CHAR:
                printf("%c\n", variable->value.char_value);
                break;
            case TYPE_STRING:
                printf("%s\n", variable->value.string_value);
                break;
            }
        }
        else if (ast_is_boolean_expression(expression))
        {
            int result;

            if (ast_evaluate_float_expression(expression))
                result = ast_evaluate_float(expression) != 0.0f;
            else
                result = ast_evaluate(expression);

            printf("%s\n", result ? "true" : "false");
        }
        else if (ast_evaluate_float_expression(expression))
            printf("%g\n", ast_evaluate_float(expression));
        else
            printf("%d\n", ast_evaluate(expression));

        break;
    }

    default:
        break;
    }
}

/* =========================================================
   SEMANTIC SYMBOL TABLE
   ========================================================= */

static void semantic_scope_push(void)
{
    SemanticScope *new_scope = calloc(1, sizeof(SemanticScope));

    if (new_scope == NULL)
    {
        fprintf(stderr, "Semantic error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    new_scope->parent = semantic_scope;
    semantic_scope = new_scope;
}

static void semantic_scope_pop(void)
{
    if (semantic_scope == NULL)
        return;

    SemanticSymbol *symbol = semantic_scope->symbols;

    while (symbol != NULL)
    {
        SemanticSymbol *next = symbol->next;
        free(symbol->name);
        free(symbol);
        symbol = next;
    }

    SemanticScope *parent = semantic_scope->parent;
    free(semantic_scope);
    semantic_scope = parent;
}

static void semantic_clear_symbols(void)
{
    while (semantic_scope != NULL)
        semantic_scope_pop();
}

static SemanticSymbol *semantic_find_current_symbol(const char *name)
{
    if (semantic_scope == NULL)
        return NULL;

    for (SemanticSymbol *symbol = semantic_scope->symbols;
         symbol != NULL; symbol = symbol->next)
    {
        if (strcmp(symbol->name, name) == 0)
            return symbol;
    }

    return NULL;
}

static SemanticSymbol *semantic_find_symbol(const char *name)
{
    for (SemanticScope *scope = semantic_scope;
         scope != NULL; scope = scope->parent)
    {
        for (SemanticSymbol *symbol = scope->symbols;
             symbol != NULL; symbol = symbol->next)
        {
            if (strcmp(symbol->name, name) == 0)
                return symbol;
        }
    }

    return NULL;
}

static void semantic_add_symbol(const char *name, DataType type,
                                int line, int initialized)
{
    SemanticSymbol *symbol = calloc(1, sizeof(SemanticSymbol));

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
    symbol->line_number = line;
    symbol->initialized = initialized;

    symbol->next = semantic_scope->symbols;
    semantic_scope->symbols = symbol;
}

static DataType semantic_get_expression_type(const ASTNode *node,
                                             DataType expected_type)
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

    case AST_INPUT:
        return expected_type;

    case AST_IDENTIFIER:
    {
        SemanticSymbol *symbol = semantic_find_symbol(node->value);

        return symbol ? symbol->type : (DataType)-1;
    }

    case AST_UNARY:
        if (strcmp(node->value, "!") == 0 &&
            semantic_get_expression_type(node->left, TYPE_BOOL) == TYPE_BOOL)
            return TYPE_BOOL;

        return (DataType)-1;

    case AST_BINARY:
    {
        const char *op = node->value;

        DataType left = semantic_get_expression_type(node->left, expected_type);
        DataType right = semantic_get_expression_type(node->right, expected_type);

        if (left == (DataType)-1 || right == (DataType)-1)
            return (DataType)-1;

        if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0)
            return left == TYPE_BOOL && right == TYPE_BOOL
                       ? TYPE_BOOL : (DataType)-1;

        if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
            strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0)
        {
            if ((left == TYPE_INT || left == TYPE_FLOAT) &&
                (right == TYPE_INT || right == TYPE_FLOAT))
                return TYPE_BOOL;

            if (left == right &&
                (left == TYPE_CHAR || left == TYPE_STRING || left == TYPE_BOOL))
                return TYPE_BOOL;

            return (DataType)-1;
        }

        if ((left != TYPE_INT && left != TYPE_FLOAT) ||
            (right != TYPE_INT && right != TYPE_FLOAT))
            return (DataType)-1;

        if (strcmp(op, "%") == 0)
            return left == TYPE_INT && right == TYPE_INT
                       ? TYPE_INT : (DataType)-1;

        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
            strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
            return left == TYPE_FLOAT || right == TYPE_FLOAT
                       ? TYPE_FLOAT : TYPE_INT;

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
        return semantic_analyze_node(node->left) &&
               semantic_analyze_node(node->right);

    if (node->type == AST_DECLARATION)
    {
        if (node->value == NULL || node->data_type == NULL)
        {
            fprintf(stderr, "Semantic error at line %d: Invalid declaration.\n",
                    node->line_number);
            return 0;
        }

        if (semantic_find_current_symbol(node->value) != NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is already declared in this scope.\n",
                    node->line_number, node->value);
            return 0;
        }

        DataType declared_type;

        if (strcmp(node->data_type, "int") == 0) declared_type = TYPE_INT;
        else if (strcmp(node->data_type, "float") == 0) declared_type = TYPE_FLOAT;
        else if (strcmp(node->data_type, "bool") == 0) declared_type = TYPE_BOOL;
        else if (strcmp(node->data_type, "char") == 0) declared_type = TYPE_CHAR;
        else if (strcmp(node->data_type, "string") == 0) declared_type = TYPE_STRING;
        else
        {
            fprintf(stderr, "Semantic error at line %d: Unknown type.\n",
                    node->line_number);
            return 0;
        }

        if (node->left == NULL)
        {
            semantic_add_symbol(node->value, declared_type,
                                node->line_number, 0);
            return 1;
        }

        if (!semantic_analyze_node(node->left))
            return 0;

        DataType initializer_type =
            semantic_get_expression_type(node->left, declared_type);

        if (initializer_type != declared_type)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Initializer type does not match declared type for '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        semantic_add_symbol(node->value, declared_type,
                            node->line_number, 1);
        return 1;
    }

    if (node->type == AST_IDENTIFIER)
    {
        SemanticSymbol *symbol = semantic_find_symbol(node->value);

        if (symbol == NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is not declared.\n",
                    node->line_number, node->value);
            return 0;
        }

        if (!symbol->initialized)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' may be used before initialization.\n",
                    node->line_number, node->value);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_ASSIGNMENT)
    {
        SemanticSymbol *symbol = semantic_find_symbol(node->value);

        if (symbol == NULL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Variable '%s' is not declared.\n",
                    node->line_number, node->value);
            return 0;
        }

        if (!semantic_analyze_node(node->right))
            return 0;

        DataType expression_type =
            semantic_get_expression_type(node->right, symbol->type);

        if (expression_type != symbol->type)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Cannot assign incompatible type to '%s'.\n",
                    node->line_number, node->value);
            return 0;
        }

        symbol->initialized = 1;
        return 1;
    }

    if (node->type == AST_PRINT)
        return semantic_analyze_node(node->left);

    if (node->type == AST_INPUT)
        return 1;

    if (node->type == AST_UNARY)
    {
        if (!semantic_analyze_node(node->left))
            return 0;

        if (semantic_get_expression_type(node, TYPE_BOOL) != TYPE_BOOL)
        {
            fprintf(stderr, "Semantic error at line %d: Invalid unary operand.\n",
                    node->line_number);
            return 0;
        }

        return 1;
    }

    if (node->type == AST_BINARY)
    {
        if (!semantic_analyze_node(node->left) ||
            !semantic_analyze_node(node->right))
            return 0;

        if (semantic_get_expression_type(node, TYPE_INT) == (DataType)-1)
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

        if (semantic_get_expression_type(node->left, TYPE_BOOL) != TYPE_BOOL)
        {
            fprintf(stderr, "Semantic error at line %d: If condition must be boolean.\n",
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

        if (semantic_get_expression_type(node->right, TYPE_BOOL) != TYPE_BOOL)
        {
            fprintf(stderr,
                    "Semantic error at line %d: Do-while condition must be boolean.\n",
                    node->line_number);
            return 0;
        }

        return 1;
    }

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
