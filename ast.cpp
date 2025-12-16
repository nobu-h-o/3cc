#include "ast.h"
#include <cstdlib>
#include <cstring>

ASTNode* ast_number(int value) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_NUMBER;
    node->data.number = value;
    return node;
}

ASTNode* ast_binary(BinaryOp op, ASTNode *left, ASTNode *right) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_BINARY_OP;
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

ASTNode* ast_variable(char *name) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_VARIABLE;
    node->data.variable = strdup(name);
    return node;
}

ASTNode* ast_assignment(char *name, ASTNode *value) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_ASSIGNMENT;
    node->data.assignment.name = strdup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* ast_return(ASTNode *value) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_RETURN;
    node->data.return_value = value;
    return node;
}

ASTNode* ast_sequence(ASTNode *first, ASTNode *second) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_SEQUENCE;
    node->data.sequence.first = first;
    node->data.sequence.second = second;
    return node;
}

ASTNode* ast_while(ASTNode *condition, ASTNode *body) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_WHILE;
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode* ast_for(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_FOR;
    node->data.for_loop.init = init;
    node->data.for_loop.condition = condition;
    node->data.for_loop.increment = increment;
    node->data.for_loop.body = body;
    return node;
}

ASTNode* ast_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_print(ASTNode *value) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_PRINT;
    node->data.print_value = value;
    return node;
}

ASTNode* ast_function_def(char *name, ParamList *params, ASTNode *body) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_FUNCTION_DEF;
    node->data.function_def.name = strdup(name);
    node->data.function_def.params = params;
    node->data.function_def.body = body;
    return node;
}

ASTNode* ast_function_call(char *name, ArgList *args) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_FUNCTION_CALL;
    node->data.function_call.name = strdup(name);
    node->data.function_call.args = args;
    return node;
}

ASTNode* ast_global_var(char *name, ASTNode *value) {
    ASTNode *node = new ASTNode;
    node->type = ASTNodeType::AST_GLOBAL_VAR;
    node->data.global_var.name = strdup(name);
    node->data.global_var.value = value;
    return node;
}

ParamList* param_list_create(char *name, ParamList *next) {
    return new ParamList(strdup(name), next);
}

ArgList* arg_list_create(ASTNode *expr, ArgList *next) {
    return new ArgList(expr, next);
}

int param_list_count(ParamList *params) {
    int count = 0;
    while (params) {
        count++;
        params = params->next;
    }
    return count;
}

int arg_list_count(ArgList *args) {
    int count = 0;
    while (args) {
        count++;
        args = args->next;
    }
    return count;
}

void param_list_free(ParamList *params) {
    while (params) {
        ParamList *next = params->next;
        free(params->name);
        delete params;
        params = next;
    }
}

void arg_list_free(ArgList *args) {
    while (args) {
        ArgList *next = args->next;
        ast_free(args->expr);
        delete args;
        args = next;
    }
}

void ast_free(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case ASTNodeType::AST_BINARY_OP:
            ast_free(node->data.binary.left);
            ast_free(node->data.binary.right);
            break;
        case ASTNodeType::AST_VARIABLE:
            free(node->data.variable);
            break;
        case ASTNodeType::AST_ASSIGNMENT:
            free(node->data.assignment.name);
            ast_free(node->data.assignment.value);
            break;
        case ASTNodeType::AST_RETURN:
            ast_free(node->data.return_value);
            break;
        case ASTNodeType::AST_SEQUENCE:
            ast_free(node->data.sequence.first);
            ast_free(node->data.sequence.second);
            break;
        case ASTNodeType::AST_WHILE:
            ast_free(node->data.while_loop.condition);
            ast_free(node->data.while_loop.body);
            break;
        case ASTNodeType::AST_FOR:
            ast_free(node->data.for_loop.init);
            ast_free(node->data.for_loop.condition);
            ast_free(node->data.for_loop.increment);
            ast_free(node->data.for_loop.body);
            break;
        case ASTNodeType::AST_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
            break;
        case ASTNodeType::AST_PRINT:
            ast_free(node->data.print_value);
            break;
        case ASTNodeType::AST_FUNCTION_DEF:
            free(node->data.function_def.name);
            param_list_free(node->data.function_def.params);
            ast_free(node->data.function_def.body);
            break;
        case ASTNodeType::AST_FUNCTION_CALL:
            free(node->data.function_call.name);
            arg_list_free(node->data.function_call.args);
            break;
        case ASTNodeType::AST_GLOBAL_VAR:
            free(node->data.global_var.name);
            ast_free(node->data.global_var.value);
            break;
        default:
            break;
    }

    delete node;
}

static GlobalVar *global_vars_list = nullptr;

static void add_global_var(const char *name, int value) {
    GlobalVar *new_var = new GlobalVar(strdup(name), value, global_vars_list);
    global_vars_list = new_var;
}

static void collect_globals(ASTNode *node) {
    if (!node) return;

    if (node->type == ASTNodeType::AST_GLOBAL_VAR) {
        // Extract constant value if it's a simple number
        int init_value = 0;
        if (node->data.global_var.value &&
            node->data.global_var.value->type == ASTNodeType::AST_NUMBER) {
            init_value = node->data.global_var.value->data.number;
        }
        add_global_var(node->data.global_var.name, init_value);
    } else if (node->type == ASTNodeType::AST_SEQUENCE) {
        collect_globals(node->data.sequence.first);
        collect_globals(node->data.sequence.second);
    }
    // Don't recurse into function bodies
}

GlobalVar* collect_global_vars(ASTNode *root) {
    global_vars_list = nullptr;
    collect_globals(root);
    return global_vars_list;
}

void global_vars_free(GlobalVar *globals) {
    while (globals) {
        GlobalVar *next = globals->next;
        free(globals->name);
        delete globals;
        globals = next;
    }
}
