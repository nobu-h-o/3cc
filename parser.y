%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "symtab.h"

void yyerror(const char *s);
int yylex(void);

ASTNode *root = nullptr;
extern SymbolTable *global_symtab;
%}


%union {
    int number;
    char *string;
    ASTNode *node;
    ParamList *params;
    ArgList *args;
}

%token NUMBER
%token IDENTIFIER
%token ASSIGN SEMICOLON COMMA
%token RETURN WHILE FOR IF ELSE PRINT
%token ADD SUB MUL DIV
%token LPAREN RPAREN LBRACE RBRACE
%token LT GT LE GE EQ NE

%left EQ NE
%left LT GT LE GE
%left ADD SUB
%left MUL DIV
%nonassoc UNARY

%type <node> program expr statement statements function_def global_decl toplevel_items toplevel_item
%type <params> param_list param_list_opt
%type <args> arg_list arg_list_opt
%type <number> NUMBER
%type <string> IDENTIFIER

%%
program:
    toplevel_items { root = $1; $$ = $1; };

toplevel_items:
    toplevel_item { $$ = $1; }
    | toplevel_items toplevel_item { $$ = ast_sequence($1, $2); };

toplevel_item:
    function_def { $$ = $1; }
    | global_decl { $$ = $1; };

function_def:
    IDENTIFIER LPAREN param_list_opt RPAREN LBRACE statements RBRACE {
        $$ = ast_function_def($1, $3, $6);
    };

global_decl:
    IDENTIFIER ASSIGN expr SEMICOLON {
        $$ = ast_global_var($1, $3);
    };

param_list_opt:
    /* empty */ { $$ = nullptr; }
    | param_list { $$ = $1; };

param_list:
    IDENTIFIER {
        $$ = param_list_create($1, nullptr);
    }
    | IDENTIFIER COMMA param_list {
        $$ = param_list_create($1, $3);
    };

arg_list_opt:
    /* empty */ { $$ = nullptr; }
    | arg_list { $$ = $1; };

arg_list:
    expr {
        $$ = arg_list_create($1, nullptr);
    }
    | expr COMMA arg_list {
        $$ = arg_list_create($1, $3);
    };

statements:
    statement { $$ = $1; }
    | statements statement { $$ = ast_sequence($1, $2); };

statement:
    IDENTIFIER ASSIGN expr SEMICOLON {
        $$ = ast_assignment($1, $3);
    }
    | RETURN expr SEMICOLON { $$ = ast_return($2); }
    | PRINT LPAREN expr RPAREN SEMICOLON { $$ = ast_print($3); }
    | expr SEMICOLON { $$ = $1; }
    | WHILE LPAREN expr RPAREN LBRACE statements RBRACE {
        $$ = ast_while($3, $6);
    }
    | FOR LPAREN statement expr SEMICOLON IDENTIFIER ASSIGN expr RPAREN LBRACE statements RBRACE {
        $$ = ast_for($3, $4, ast_assignment($6, $8), $11);
    }
    | IF LPAREN expr RPAREN LBRACE statements RBRACE {
        $$ = ast_if($3, $6, nullptr);
    }
    | IF LPAREN expr RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE {
        $$ = ast_if($3, $6, $10);
    };

expr:
    NUMBER { $$ = ast_number($1); }
    | IDENTIFIER {
        $$ = ast_variable($1);
    }
    | IDENTIFIER LPAREN arg_list_opt RPAREN {
        $$ = ast_function_call($1, $3);
    }
    | expr ADD expr { $$ = ast_binary(BinaryOp::OP_ADD, $1, $3); }
    | expr SUB expr { $$ = ast_binary(BinaryOp::OP_SUB, $1, $3); }
    | expr MUL expr { $$ = ast_binary(BinaryOp::OP_MUL, $1, $3); }
    | expr DIV expr { $$ = ast_binary(BinaryOp::OP_DIV, $1, $3); }
    | expr LT expr { $$ = ast_binary(BinaryOp::OP_LT, $1, $3); }
    | expr GT expr { $$ = ast_binary(BinaryOp::OP_GT, $1, $3); }
    | expr LE expr { $$ = ast_binary(BinaryOp::OP_LE, $1, $3); }
    | expr GE expr { $$ = ast_binary(BinaryOp::OP_GE, $1, $3); }
    | expr EQ expr { $$ = ast_binary(BinaryOp::OP_EQ, $1, $3); }
    | expr NE expr { $$ = ast_binary(BinaryOp::OP_NE, $1, $3); }
    | LPAREN expr RPAREN { $$ = $2; };
%%

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}
