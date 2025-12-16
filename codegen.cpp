#include "codegen.h"
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <iostream>
#include <cstring>

SymbolTable *global_symtab = nullptr;

CodeGenerator::CodeGenerator() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("3cc", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    current_function = nullptr;
    current_return_block = nullptr;
    return_value_alloca = nullptr;
    printf_func = nullptr;

    create_printf_declaration();
}

void CodeGenerator::create_printf_declaration() {
    // Declare printf: i32 @printf(i8*, ...)
    llvm::FunctionType *printf_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        llvm::PointerType::getUnqual(*context),
        true  // varargs
    );

    printf_func = llvm::Function::Create(
        printf_type,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );
}

llvm::AllocaInst* CodeGenerator::create_entry_block_alloca(
    llvm::Function *func, const std::string &var_name) {
    llvm::IRBuilder<> tmp_builder(&func->getEntryBlock(), func->getEntryBlock().begin());
    return tmp_builder.CreateAlloca(llvm::Type::getInt32Ty(*context), nullptr, var_name);
}

bool CodeGenerator::is_global_var(const std::string &name) const {
    return global_vars.find(name) != global_vars.end();
}

llvm::Value* CodeGenerator::codegen_expr(ASTNode *node) {
    if (!node) return nullptr;

    switch (node->type) {
        case ASTNodeType::AST_NUMBER:
            return llvm::ConstantInt::get(*context, llvm::APInt(32, node->data.number, true));

        case ASTNodeType::AST_VARIABLE: {
            std::string name = node->data.variable;

            // Check if it's a global variable
            if (is_global_var(name)) {
                auto global = global_vars[name];
                return builder->CreateLoad(llvm::Type::getInt32Ty(*context), global, name);
            }

            // Local variable
            llvm::AllocaInst *alloca = named_values[name];
            if (!alloca) {
                // Variable not found, create it
                alloca = create_entry_block_alloca(current_function, name);
                named_values[name] = alloca;
            }
            return builder->CreateLoad(llvm::Type::getInt32Ty(*context), alloca, name);
        }

        case ASTNodeType::AST_BINARY_OP: {
            llvm::Value *left = codegen_expr(node->data.binary.left);
            llvm::Value *right = codegen_expr(node->data.binary.right);

            if (!left || !right) return nullptr;

            switch (node->data.binary.op) {
                case BinaryOp::OP_ADD:
                    return builder->CreateAdd(left, right, "addtmp");
                case BinaryOp::OP_SUB:
                    return builder->CreateSub(left, right, "subtmp");
                case BinaryOp::OP_MUL:
                    return builder->CreateMul(left, right, "multmp");
                case BinaryOp::OP_DIV:
                    return builder->CreateSDiv(left, right, "divtmp");
                case BinaryOp::OP_LT:
                    return builder->CreateZExt(
                        builder->CreateICmpSLT(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                case BinaryOp::OP_GT:
                    return builder->CreateZExt(
                        builder->CreateICmpSGT(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                case BinaryOp::OP_LE:
                    return builder->CreateZExt(
                        builder->CreateICmpSLE(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                case BinaryOp::OP_GE:
                    return builder->CreateZExt(
                        builder->CreateICmpSGE(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                case BinaryOp::OP_EQ:
                    return builder->CreateZExt(
                        builder->CreateICmpEQ(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                case BinaryOp::OP_NE:
                    return builder->CreateZExt(
                        builder->CreateICmpNE(left, right, "cmptmp"),
                        llvm::Type::getInt32Ty(*context), "booltmp");
                default:
                    return nullptr;
            }
        }

        case ASTNodeType::AST_FUNCTION_CALL: {
            std::string name = node->data.function_call.name;

            // Look up the function in the module
            llvm::Function *callee = module->getFunction(name);
            if (!callee) {
                std::cerr << "Unknown function referenced: " << name << std::endl;
                return nullptr;
            }

            // Generate code for arguments
            std::vector<llvm::Value*> args_values;
            ArgList *arg = node->data.function_call.args;
            while (arg) {
                llvm::Value *arg_val = codegen_expr(arg->expr);
                if (!arg_val) return nullptr;
                args_values.push_back(arg_val);
                arg = arg->next;
            }

            return builder->CreateCall(callee, args_values, "calltmp");
        }

        default:
            return nullptr;
    }
}

void CodeGenerator::codegen_stmt(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case ASTNodeType::AST_ASSIGNMENT: {
            std::string name = node->data.assignment.name;
            llvm::Value *val = codegen_expr(node->data.assignment.value);
            if (!val) return;

            // Check if it's a global variable
            if (is_global_var(name)) {
                builder->CreateStore(val, global_vars[name]);
                return;
            }

            // Local variable
            llvm::AllocaInst *alloca = named_values[name];
            if (!alloca) {
                // Variable not found, create it
                alloca = create_entry_block_alloca(current_function, name);
                named_values[name] = alloca;
            }
            builder->CreateStore(val, alloca);
            break;
        }

        case ASTNodeType::AST_RETURN: {
            llvm::Value *ret_val = codegen_expr(node->data.return_value);
            if (ret_val && return_value_alloca) {
                builder->CreateStore(ret_val, return_value_alloca);
                builder->CreateBr(current_return_block);
            }
            break;
        }

        case ASTNodeType::AST_SEQUENCE:
            codegen_stmt(node->data.sequence.first);
            codegen_stmt(node->data.sequence.second);
            break;

        case ASTNodeType::AST_WHILE: {
            llvm::BasicBlock *loop_block = llvm::BasicBlock::Create(*context, "loop", current_function);
            llvm::BasicBlock *body_block = llvm::BasicBlock::Create(*context, "loopbody", current_function);
            llvm::BasicBlock *after_block = llvm::BasicBlock::Create(*context, "afterloop", current_function);

            // Jump to loop condition
            builder->CreateBr(loop_block);
            builder->SetInsertPoint(loop_block);

            // Evaluate condition
            llvm::Value *cond = codegen_expr(node->data.while_loop.condition);
            if (!cond) return;

            // Convert condition to boolean
            llvm::Value *cond_bool = builder->CreateICmpNE(
                cond,
                llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)),
                "loopcond"
            );

            builder->CreateCondBr(cond_bool, body_block, after_block);

            // Emit loop body
            builder->SetInsertPoint(body_block);
            codegen_stmt(node->data.while_loop.body);
            builder->CreateBr(loop_block);

            // Continue after loop
            builder->SetInsertPoint(after_block);
            break;
        }

        case ASTNodeType::AST_FOR: {
            // Execute init
            codegen_stmt(node->data.for_loop.init);

            llvm::BasicBlock *loop_block = llvm::BasicBlock::Create(*context, "forloop", current_function);
            llvm::BasicBlock *body_block = llvm::BasicBlock::Create(*context, "forbody", current_function);
            llvm::BasicBlock *after_block = llvm::BasicBlock::Create(*context, "afterfor", current_function);

            // Jump to loop condition
            builder->CreateBr(loop_block);
            builder->SetInsertPoint(loop_block);

            // Evaluate condition
            llvm::Value *cond = codegen_expr(node->data.for_loop.condition);
            if (!cond) return;

            // Convert condition to boolean
            llvm::Value *cond_bool = builder->CreateICmpNE(
                cond,
                llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)),
                "forcond"
            );

            builder->CreateCondBr(cond_bool, body_block, after_block);

            // Emit loop body
            builder->SetInsertPoint(body_block);
            codegen_stmt(node->data.for_loop.body);
            codegen_stmt(node->data.for_loop.increment);
            builder->CreateBr(loop_block);

            // Continue after loop
            builder->SetInsertPoint(after_block);
            break;
        }

        case ASTNodeType::AST_IF: {
            llvm::Value *cond = codegen_expr(node->data.if_stmt.condition);
            if (!cond) return;

            // Convert condition to boolean
            llvm::Value *cond_bool = builder->CreateICmpNE(
                cond,
                llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)),
                "ifcond"
            );

            llvm::BasicBlock *then_block = llvm::BasicBlock::Create(*context, "then", current_function);
            llvm::BasicBlock *else_block = node->data.if_stmt.else_branch ?
                llvm::BasicBlock::Create(*context, "else", current_function) : nullptr;
            llvm::BasicBlock *merge_block = llvm::BasicBlock::Create(*context, "ifcont", current_function);

            // Branch based on condition
            if (else_block) {
                builder->CreateCondBr(cond_bool, then_block, else_block);
            } else {
                builder->CreateCondBr(cond_bool, then_block, merge_block);
            }

            // Emit then block
            builder->SetInsertPoint(then_block);
            codegen_stmt(node->data.if_stmt.then_branch);
            if (!builder->GetInsertBlock()->getTerminator()) {
                builder->CreateBr(merge_block);
            }

            // Emit else block if it exists
            if (else_block) {
                builder->SetInsertPoint(else_block);
                codegen_stmt(node->data.if_stmt.else_branch);
                if (!builder->GetInsertBlock()->getTerminator()) {
                    builder->CreateBr(merge_block);
                }
            }

            // Continue with merge block
            builder->SetInsertPoint(merge_block);
            break;
        }

        case ASTNodeType::AST_PRINT: {
            llvm::Value *val = codegen_expr(node->data.print_value);
            if (!val) return;

            // Create format string "%d\n"
            llvm::Value *format_str = builder->CreateGlobalString("%d\n");

            // Call printf
            builder->CreateCall(printf_func, {format_str, val});
            break;
        }

        case ASTNodeType::AST_FUNCTION_DEF:
            codegen_function_def(node);
            break;

        case ASTNodeType::AST_GLOBAL_VAR:
            // Global variables are handled separately
            break;

        default:
            // For expressions used as statements
            codegen_expr(node);
            break;
    }
}

void CodeGenerator::codegen_function_def(ASTNode *node) {
    std::string func_name = node->data.function_def.name;
    ParamList *params = node->data.function_def.params;

    // Count parameters
    int param_count = param_list_count(params);

    // Create function type
    std::vector<llvm::Type*> param_types(param_count, llvm::Type::getInt32Ty(*context));
    llvm::FunctionType *func_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        param_types,
        false
    );

    // Create function
    llvm::Function *func = llvm::Function::Create(
        func_type,
        llvm::Function::ExternalLinkage,
        func_name,
        module.get()
    );

    // Set parameter names
    ParamList *param = params;
    for (auto &arg : func->args()) {
        if (param) {
            arg.setName(param->name);
            param = param->next;
        }
    }

    // Create entry block
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entry);

    // Save previous context
    auto prev_named_values = named_values;
    auto prev_function = current_function;
    auto prev_return_block = current_return_block;
    auto prev_return_alloca = return_value_alloca;

    // Set current function context
    current_function = func;
    named_values.clear();

    // Create return block and alloca for return value
    current_return_block = llvm::BasicBlock::Create(*context, "return", func);
    return_value_alloca = create_entry_block_alloca(func, "retval");

    // Initialize return value to 0
    builder->CreateStore(llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)), return_value_alloca);

    // Allocate space for parameters and store their values
    param = params;
    for (auto &arg : func->args()) {
        if (param) {
            llvm::AllocaInst *alloca = create_entry_block_alloca(func, param->name);
            builder->CreateStore(&arg, alloca);
            named_values[param->name] = alloca;
            param = param->next;
        }
    }

    // Generate function body
    codegen_stmt(node->data.function_def.body);

    // If we haven't branched to return block yet, do it now
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(current_return_block);
    }

    // Generate return block
    builder->SetInsertPoint(current_return_block);
    llvm::Value *ret_val = builder->CreateLoad(llvm::Type::getInt32Ty(*context), return_value_alloca, "retval");
    builder->CreateRet(ret_val);

    // Verify function
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        std::cerr << "Error in function " << func_name << std::endl;
    }

    // Restore previous context
    named_values = prev_named_values;
    current_function = prev_function;
    current_return_block = prev_return_block;
    return_value_alloca = prev_return_alloca;
}

void CodeGenerator::generate_program(ASTNode *root, GlobalVar *globals) {
    // Create global variables
    GlobalVar *global = globals;
    while (global) {
        llvm::GlobalVariable *gv = new llvm::GlobalVariable(
            *module,
            llvm::Type::getInt32Ty(*context),
            false,  // not constant
            llvm::GlobalValue::ExternalLinkage,
            llvm::ConstantInt::get(*context, llvm::APInt(32, global->value, true)),
            global->name
        );
        global_vars[global->name] = gv;
        global = global->next;
    }

    // Generate code for all functions
    codegen_stmt(root);

    // Verify module
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error in module" << std::endl;
    }
}

void CodeGenerator::output_ir(const std::string &filename) {
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec);

    if (ec) {
        std::cerr << "Could not open file: " << ec.message() << std::endl;
        return;
    }

    module->print(dest, nullptr);
}

void CodeGenerator::optimize_module() {
    // Create a function pass manager
    llvm::legacy::FunctionPassManager fpm(module.get());

    // Add optimization passes (similar to -O2/-O3)
    fpm.add(llvm::createPromoteMemoryToRegisterPass());  // mem2reg: promote allocas to registers
    fpm.add(llvm::createInstructionCombiningPass());     // Combine instructions
    fpm.add(llvm::createReassociatePass());              // Reassociate expressions
    fpm.add(llvm::createGVNPass());                      // Global Value Numbering (removes redundancy)
    fpm.add(llvm::createCFGSimplificationPass());        // Simplify control flow graph
    fpm.add(llvm::createDeadCodeEliminationPass());      // Remove dead code

    fpm.doInitialization();

    // Run the optimization passes on each function
    for (auto &func : *module) {
        fpm.run(func);
    }

    fpm.doFinalization();
}

void CodeGenerator::output_object_file(const std::string &filename) {
    // Initialize native target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    auto target_triple_str = llvm::sys::getDefaultTargetTriple();
    llvm::Triple target_triple(target_triple_str);
    module->setTargetTriple(target_triple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple_str, error);

    if (!target) {
        std::cerr << error << std::endl;
        return;
    }

    auto cpu = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto target_machine = target->createTargetMachine(
        target_triple, cpu, features, opt, llvm::Reloc::PIC_);

    module->setDataLayout(target_machine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

    if (ec) {
        std::cerr << "Could not open file: " << ec.message() << std::endl;
        return;
    }

    llvm::legacy::PassManager pass;
    auto file_type = llvm::CodeGenFileType::ObjectFile;

    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
        std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
        return;
    }

    pass.run(*module);
    dest.flush();
}

// C-style interface
extern "C" {
    void codegen_program(ASTNode *root, GlobalVar *globals) {
        // This is called from the C-style main
        // We'll handle this differently in main.cpp
    }
}
