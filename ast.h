#ifndef AST_H
#define AST_H


typedef enum {
    AST_PROGRAM,
    AST_INT,
    AST_FLOAT,
    AST_IDENTIFIER,
    AST_BINARY,
    AST_BOOL,
    AST_PRINT,
    AST_DECLARATION,
    AST_CHAR,
    AST_ASSIGNMENT
}ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    char* value;

    struct ASTNode* left;
    struct ASTNode* right;
}ASTNode;

typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_BOOL
}DataType;

typedef struct Variable
{
    char* name;
    DataType type;

    union {
        int int_value;
        float float_value;
        int bool_value;
        char char_value;
        char* string_value;
    } value;

    struct Variable* next;
} Variable;

ASTNode *ast_create(ASTNodeType type, const char *value, 
    ASTNode* left,ASTNode* right);

void ast_print(const ASTNode* node, int depth) ;

void ast_free(ASTNode *node);
int ast_evaluate(const ASTNode* node);
float ast_evaluate_float(const ASTNode* node);
//int ast_evaluate_float_expression(const ASTNode* node);

void ast_set_variable(const char* name, DataType type, const char* value);
Variable* ast_get_variable(const char* name);

void ast_export(const ASTNode* node);
void ast_execute(const ASTNode* node);
void ast_free_variables(void);

#endif

