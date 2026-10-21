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
    AST_CHAR
}ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    char* value;

    struct ASTNode* left;
    struct ASTNode* right;
}ASTNode;

typedef struct Variable
{
    char* name;
    int value;
    struct Variable* next;
} Variable;

ASTNode *ast_create(ASTNodeType type, const char *value, 
    ASTNode* left,ASTNode* right);

void ast_print(const ASTNode* node, int depth) ;

void ast_free(ASTNode *node);
int ast_evaluate(const ASTNode* node);

void ast_set_variable(const char* name, int value);
int ast_get_variable(const char* name);

#endif

