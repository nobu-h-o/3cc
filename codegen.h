#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symtab.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <map>
#include <string>

class CodeGenerator {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    std::map<std::string, llvm::AllocaInst*> named_values;
    std::map<std::string, llvm::GlobalVariable*> global_vars;

    llvm::Function *current_function;
    llvm::BasicBlock *current_return_block;
    llvm::AllocaInst *return_value_alloca;

    llvm::Function *printf_func;

    void create_printf_declaration();
    llvm::AllocaInst* create_entry_block_alloca(llvm::Function *func, const std::string &var_name);

    llvm::Value* codegen_expr(ASTNode *node);
    void codegen_stmt(ASTNode *node);
    void codegen_function_def(ASTNode *node);

    bool is_global_var(const std::string &name) const;

public:
    CodeGenerator();
    ~CodeGenerator() = default;

    void generate_program(ASTNode *root, GlobalVar *globals);
    void optimize_module();
    void output_ir(const std::string &filename);
    void output_object_file(const std::string &filename);
};

// C-style interface for compatibility
extern "C" {
    void codegen_program(ASTNode *root, GlobalVar *globals);
}

extern SymbolTable *global_symtab;

#endif /* CODEGEN_H */
