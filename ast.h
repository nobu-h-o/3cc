#ifndef AST_H
#define AST_H

#include <string>

enum class ASTNodeType {
    AST_NUMBER,
    AST_BINARY_OP,
    AST_VARIABLE,
    AST_ASSIGNMENT,
    AST_RETURN,
    AST_SEQUENCE,
    AST_WHILE,
    AST_FOR,
    AST_IF,
    AST_PRINT,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_GLOBAL_VAR,
};

enum class BinaryOp {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE,
};

struct ParamList {
    char *name;
    ParamList *next;

    ParamList(char *n, ParamList *nxt) : name(n), next(nxt) {}
};

struct ArgList {
    struct ASTNode *expr;
    ArgList *next;

    ArgList(ASTNode *e, ArgList *nxt) : expr(e), next(nxt) {}
};

struct GlobalVar {
    char *name;
    int value;
    GlobalVar *next;

    GlobalVar(char *n, int v, GlobalVar *nxt) : name(n), value(v), next(nxt) {}
};

struct ASTNode {
    ASTNodeType type;
    union {
        int number;
        struct {
            BinaryOp op;
            ASTNode *left;
            ASTNode *right;
        } binary;
        char *variable;
        struct {
            char *name;
            ASTNode *value;
        } assignment;
        ASTNode *return_value;
        struct {
            ASTNode *first;
            ASTNode *second;
        } sequence;
        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_loop;
        struct {
            ASTNode *init;
            ASTNode *condition;
            ASTNode *increment;
            ASTNode *body;
        } for_loop;
        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;
        ASTNode *print_value;
        struct {
            char *name;
            ParamList *params;
            ASTNode *body;
        } function_def;
        struct {
            char *name;
            ArgList *args;
        } function_call;
        struct {
            char *name;
            ASTNode *value;
        } global_var;
    } data;
};

// AST node creation functions (C-style for bison compatibility)
ASTNode* ast_number(int value);
ASTNode* ast_binary(BinaryOp op, ASTNode *left, ASTNode *right);
ASTNode* ast_variable(char *name);
ASTNode* ast_assignment(char *name, ASTNode *value);
ASTNode* ast_return(ASTNode *value);
ASTNode* ast_sequence(ASTNode *first, ASTNode *second);
ASTNode* ast_while(ASTNode *condition, ASTNode *body);
ASTNode* ast_for(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body);
ASTNode* ast_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
ASTNode* ast_print(ASTNode *value);
ASTNode* ast_function_def(char *name, ParamList *params, ASTNode *body);
ASTNode* ast_function_call(char *name, ArgList *args);
ASTNode* ast_global_var(char *name, ASTNode *value);

// List helpers
ParamList* param_list_create(char *name, ParamList *next);
ArgList* arg_list_create(ASTNode *expr, ArgList *next);
int param_list_count(ParamList *params);
int arg_list_count(ArgList *args);
void param_list_free(ParamList *params);
void arg_list_free(ArgList *args);
void ast_free(ASTNode *node);

// Global variable collection
GlobalVar* collect_global_vars(ASTNode *root);
void global_vars_free(GlobalVar *globals);

#endif /* AST_H */
