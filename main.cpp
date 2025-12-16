#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "codegen.h"
#include "symtab.h"

extern void yy_scan_string(const char *str);
extern int yyparse();
extern ASTNode *root;

static bool check_main_exists(ASTNode *node) {
    if (!node) return false;

    if (node->type == ASTNodeType::AST_FUNCTION_DEF) {
        if (strcmp(node->data.function_def.name, "main") == 0) {
            return true;
        }
    } else if (node->type == ASTNodeType::AST_SEQUENCE) {
        return check_main_exists(node->data.sequence.first) ||
               check_main_exists(node->data.sequence.second);
    }

    return false;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <source_code> [output_file]" << std::endl;
        return 1;
    }

    std::string output_file = "output.o";
    if (argc == 3) {
        output_file = argv[2];
    }

    global_symtab = symtab_create();

    yy_scan_string(argv[1]);
    yyparse();

    if (root) {
        // Check if main function exists
        if (root->type == ASTNodeType::AST_FUNCTION_DEF &&
            strcmp(root->data.function_def.name, "main") == 0) {
            // Single main function - OK
        } else if (root->type == ASTNodeType::AST_SEQUENCE ||
                   root->type == ASTNodeType::AST_FUNCTION_DEF) {
            // Multiple functions or single non-main function - check if main exists
            if (!check_main_exists(root)) {
                std::cerr << "Error: main() function is required" << std::endl;
                ast_free(root);
                symtab_free(global_symtab);
                return 1;
            }
        }

        // Collect global variables
        GlobalVar *globals = collect_global_vars(root);

        // Generate code using LLVM
        CodeGenerator codegen;
        codegen.generate_program(root, globals);

        // Run LLVM optimization passes
        codegen.optimize_module();

        // Output LLVM IR to .ll file for inspection
        std::string ir_file = output_file;
        size_t pos = ir_file.rfind('.');
        if (pos != std::string::npos) {
            ir_file = ir_file.substr(0, pos) + ".ll";
        } else {
            ir_file += ".ll";
        }
        codegen.output_ir(ir_file);

        // Output object file
        codegen.output_object_file(output_file);

        std::cout << "Compilation successful!" << std::endl;
        std::cout << "LLVM IR: " << ir_file << std::endl;
        std::cout << "Object file: " << output_file << std::endl;

        // Free globals
        global_vars_free(globals);

        ast_free(root);
    }

    symtab_free(global_symtab);

    return 0;
}
