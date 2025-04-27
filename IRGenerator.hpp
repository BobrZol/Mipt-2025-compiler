#pragma once
#include "Ast.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

class IRGenerator {
  llvm::LLVMContext& context;
  llvm::Module* module;
  llvm::IRBuilder<> builder;
  std::shared_ptr<TypeScope> current_scope;

public:
  IRGenerator(llvm::LLVMContext& ctx, llvm::Module* mod)
      : context(ctx), module(mod), builder(ctx) {
    current_scope = std::make_shared<TypeScope>();
  }

  void Generate(Root& root);

  void Interpret(Root& ref, std::shared_ptr<TypeScope> scope);
  llvm::Value* Interpret(Number& ref, std::shared_ptr<TypeScope> scope);
  void Interpret(Print& ref, std::shared_ptr<TypeScope> scope);
  void Interpret(Condition& ref, std::shared_ptr<TypeScope> scope);
  void Interpret(Declare& ref, std::shared_ptr<TypeScope> scope);
  void Interpret(Assignment& ref, std::shared_ptr<TypeScope> scope);
  llvm::Value* Interpret(Variable& ref, std::shared_ptr<TypeScope> scope);
  llvm::Value* Interpret(BinOp& ref, std::shared_ptr<TypeScope> scope);

  llvm::Type* getIntType() { return builder.getInt32Ty(); }
};
