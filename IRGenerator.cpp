#include "IRGenerator.hpp"
#include "Ast.hpp"
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;

void IRGenerator::Generate(Root& root) {
  FunctionType* funcType = FunctionType::get(Type::getInt32Ty(context), false);
  Function* mainFunc =
      Function::Create(funcType, Function::ExternalLinkage, "main", module);

  BasicBlock* entry = BasicBlock::Create(context, "entry", mainFunc);
  builder.SetInsertPoint(entry);

  root.InterpretStmt(globalScope, *this);

  builder.CreateRet(ConstantInt::get(Type::getInt32Ty(context), 0));
  verifyModule(*module, &llvm::errs());
}

void IRGenerator::Interpret(Root& ref, std::shared_ptr<Scope> scope) {
  ref.head->InterpretStmt(scope, *this);
  if (ref.next)
    ref.next->InterpretStmt(scope, *this);
}

Value* IRGenerator::Interpret(Number& ref, std::shared_ptr<Scope> scope) {
  return ConstantInt::get(Type::getInt32Ty(context), ref.value);
}

void IRGenerator::Interpret(Print& ref, std::shared_ptr<Scope> scope) {
  Value* val = ref.expr_->InterpretExpr(scope, *this);

  PointerType* printfArgType = PointerType::getUnqual(Type::getInt8Ty(context));

  FunctionType* printfType =
      FunctionType::get(Type::getInt32Ty(context), {printfArgType}, true);

  FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

  Value* formatStr = builder.CreateGlobalStringPtr("%d\n");
  builder.CreateCall(printfFunc, {formatStr, val});
}

void IRGenerator::Interpret(Condition& ref, std::shared_ptr<Scope> scope) {
  Value* condVal = ref.condition_->InterpretExpr(scope, *this);
  if (condVal->getType() != builder.getInt1Ty()) {
    condVal = builder.CreateICmpNE(
        condVal, ConstantInt::get(condVal->getType(), 0), "bool_cast");
  }

  Function* func = builder.GetInsertBlock()->getParent();
  BasicBlock* thenBB = BasicBlock::Create(context, "then", func);
  BasicBlock* elseBB = BasicBlock::Create(context, "else");
  BasicBlock* mergeBB = BasicBlock::Create(context, "merge");

  builder.CreateCondBr(condVal, thenBB, elseBB);

  builder.SetInsertPoint(thenBB);
  ref.then_stmt->InterpretStmt(scope, *this);
  builder.CreateBr(mergeBB);

  func->insert(func->end(), elseBB);
  builder.SetInsertPoint(elseBB);
  ref.else_stmt->InterpretStmt(scope, *this);
  builder.CreateBr(mergeBB);

  func->insert(func->end(), mergeBB);
  builder.SetInsertPoint(mergeBB);
}

void IRGenerator::Interpret(Declare& ref, std::shared_ptr<Scope> scope) {
  AllocaInst* alloca =
      builder.CreateAlloca(Type::getInt32Ty(context), nullptr, ref.name_);
  symbol_table[ref.name_] = alloca;
}

void IRGenerator::Interpret(Assignment& ref, std::shared_ptr<Scope> scope) {
  Value* val = ref.expr_->InterpretExpr(scope, *this);
  Value* ptr = symbol_table.at(ref.name_);
  builder.CreateStore(val, ptr);
}

Value* IRGenerator::Interpret(Variable& ref, std::shared_ptr<Scope> scope) {
  return builder.CreateLoad(Type::getInt32Ty(context), symbol_table[ref.name_],
                            ref.name_);
}

Value* IRGenerator::Interpret(BinOp& ref, std::shared_ptr<Scope> scope) {
  Value* L = ref.left_->InterpretExpr(scope, *this);
  Value* R = ref.right_->InterpretExpr(scope, *this);

  if (L->getType() != R->getType()) {
    if (L->getType()->isIntegerTy(1)) {
      L = builder.CreateIntCast(L, R->getType(), false, "casttmp");
    } else if (R->getType()->isIntegerTy(1)) {
      R = builder.CreateIntCast(R, L->getType(), false, "casttmp");
    } else {
      R = builder.CreateIntCast(R, L->getType(), true, "casttmp");
    }
  }

  if (ref.op_ == "==") {
    return builder.CreateICmpEQ(L, R, "eqtmp");
  }

  if (ref.op_ == "+")
    return builder.CreateAdd(L, R, "addtmp");
  if (ref.op_ == "-")
    return builder.CreateSub(L, R, "subtmp");
  if (ref.op_ == "*")
    return builder.CreateMul(L, R, "multmp");
  if (ref.op_ == "/")
    return builder.CreateSDiv(L, R, "divtmp");
  if (ref.op_ == "==")
    return builder.CreateICmpEQ(L, R, "eqtmp");

  return nullptr;
}
