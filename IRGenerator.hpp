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
  std::unordered_map<std::string, llvm::Value*> symbol_table;
  InterpreterBase base;

public:
  IRGenerator(llvm::LLVMContext& ctx, llvm::Module* mod)
      : context(ctx), module(mod), builder(ctx), base(InterpreterBase()) {}

  void Generate(Root& root);

  void Interpret(Root& ref, std::shared_ptr<Scope> scope);
  llvm::Value* Interpret(Number& ref, std::shared_ptr<Scope> scope);
  void Interpret(Print& ref, std::shared_ptr<Scope> scope);
  void Interpret(Condition& ref, std::shared_ptr<Scope> scope);
  void Interpret(Declare& ref, std::shared_ptr<Scope> scope);
  void Interpret(Assignment& ref, std::shared_ptr<Scope> scope);
  llvm::Value* Interpret(Variable& ref, std::shared_ptr<Scope> scope);
  llvm::Value* Interpret(BinOp& ref, std::shared_ptr<Scope> scope);

  llvm::Type* getIntType() { return builder.getInt32Ty(); }
};
