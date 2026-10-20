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
    AST_DECLARATION
}ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    char* value;

    struct ASTNode* left;
    struct ASTNode* right;
}ASTNode;

ASTNode *ast_create(ASTNodeType type, const char *value, 
    ASTNode* left,ASTNode* right);

void ast_print(const ASTNode* node, int depth) ;

void ast_free(ASTNode *node);

#endif

