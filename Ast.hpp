#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <unordered_map>
#include <variant>

class InterpreterBase;
class VisitorInterpret;
class IRGenerator;
class TypeChecker;
class VisitorTypeCheck;

// class Scope {
// public:
//   Scope(std::shared_ptr<Scope> parent = nullptr);
//   void SetVar(const std::string& name, int value);
//   int GetVar(const std::string& name);
//   void PrintAst(std::ofstream& out_file);

// private:
//   std::unordered_map<std::string, int> variables;
//   std::shared_ptr<Scope> parent_;
// };

// inline std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

class TypeScope {
public:
  TypeScope(std::shared_ptr<TypeScope> parent = nullptr) : parent_(parent) {}
  void SetVar(const std::string& name, const std::string& type) {
    variables[name] = type;
  }
  bool HasVar(const std::string& name) const {
    return variables.find(name) != variables.end();
  }
  std::string GetVarType(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end())
      return it->second;
    if (parent_)
      return parent_->GetVarType(name);
    return "";
  }
  void SetValue(const std::string& name, llvm::Value* value) {
    values[name] = value;
  }
  llvm::Value* GetValue(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end())
      return it->second;
    if (parent_)
      return parent_->GetValue(name);
    return nullptr;
  }

private:
  std::unordered_map<std::string, llvm::Value*> values;
  std::unordered_map<std::string, std::string> variables;
  std::shared_ptr<TypeScope> parent_;
};

class Expr {
public:
  virtual int InterpretExpr(std::shared_ptr<TypeScope> scope,
                            const VisitorInterpret& v) = 0;
  virtual llvm::Value* InterpretExpr(std::shared_ptr<TypeScope> scope,
                                     IRGenerator& v) = 0;
  virtual void PrintAst(std::ofstream& out_file) = 0;
  virtual std::string TypeCheckExpr(std::shared_ptr<TypeScope> scope,
                                    VisitorTypeCheck& v) = 0;
  virtual ~Expr() = default;
};

class Stmt {
public:
  virtual void InterpretStmt(std::shared_ptr<TypeScope> scope,
                             const VisitorInterpret& v) = 0;
  virtual void InterpretStmt(std::shared_ptr<TypeScope> scope,
                             IRGenerator& v) = 0;
  virtual void PrintAst(std::ofstream& out_file) = 0;
  virtual void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                             VisitorTypeCheck& v) = 0;
  virtual ~Stmt() = default;
};

class Root : public Stmt {
public:
  Root(Stmt* head);
  Root(Stmt* stmt, Root* next);
  void InterpretStmt(std::shared_ptr<TypeScope> scope,
                     const VisitorInterpret& v) override;
  void InterpretStmt(std::shared_ptr<TypeScope> scope, IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                     VisitorTypeCheck& v) override;

  ~Root();

private:
  Stmt* head;
  Root* next;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Boolean : public Expr {
public:
  Boolean(bool value);
  int InterpretExpr(std::shared_ptr<TypeScope> scope,
                    const VisitorInterpret& v) override;
  llvm::Value* InterpretExpr(std::shared_ptr<TypeScope> scope,
                             IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  std::string TypeCheckExpr(std::shared_ptr<TypeScope> scope,
                            VisitorTypeCheck& v) override;

private:
  bool value;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Number : public Expr {
public:
  Number(int value);
  int InterpretExpr(std::shared_ptr<TypeScope> scope,
                    const VisitorInterpret& v) override;
  llvm::Value* InterpretExpr(std::shared_ptr<TypeScope> scope,
                             IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  std::string TypeCheckExpr(std::shared_ptr<TypeScope> scope,
                            VisitorTypeCheck& v) override;

private:
  int value;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Print : public Stmt {
public:
  Print(std::unique_ptr<Expr> expr);
  void InterpretStmt(std::shared_ptr<TypeScope> scope,
                     const VisitorInterpret& v) override;
  void InterpretStmt(std::shared_ptr<TypeScope> scope, IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                     VisitorTypeCheck& v) override;

private:
  std::unique_ptr<Expr> expr_;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Condition : public Stmt {
public:
  Condition(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_stmt,
            std::unique_ptr<Stmt> else_stmt);
  void InterpretStmt(std::shared_ptr<TypeScope> scope,
                     const VisitorInterpret& v) override;
  void InterpretStmt(std::shared_ptr<TypeScope> scope, IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                     VisitorTypeCheck& v) override;

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> then_stmt;
  std::unique_ptr<Stmt> else_stmt;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Declare : public Stmt {
public:
  Declare(const std::string& name, const std::string type_);
  void InterpretStmt(std::shared_ptr<TypeScope> scope,
                     const VisitorInterpret& v) override;
  void InterpretStmt(std::shared_ptr<TypeScope> scope, IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                     VisitorTypeCheck& v) override;

private:
  std::string name_;
  std::variant<int, bool> value_;
  std::string type_;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Assignment : public Stmt {
public:
  Assignment(const std::string& name, std::unique_ptr<Expr> expr);
  void InterpretStmt(std::shared_ptr<TypeScope> scope,
                     const VisitorInterpret& v) override;
  void InterpretStmt(std::shared_ptr<TypeScope> scope, IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  void TypeCheckStmt(std::shared_ptr<TypeScope> scope,
                     VisitorTypeCheck& v) override;

private:
  std::string name_;
  std::unique_ptr<Expr> expr_;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class Variable : public Expr {
public:
  Variable(const std::string& name);
  int InterpretExpr(std::shared_ptr<TypeScope> scope,
                    const VisitorInterpret& v) override;
  llvm::Value* InterpretExpr(std::shared_ptr<TypeScope> scope,
                             IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  std::string TypeCheckExpr(std::shared_ptr<TypeScope> scope,
                            VisitorTypeCheck& v) override;

private:
  std::string name_;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class BinOp : public Expr {
public:
  BinOp(const std::string& op, std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right);
  int InterpretExpr(std::shared_ptr<TypeScope> scope,
                    const VisitorInterpret& v) override;
  llvm::Value* InterpretExpr(std::shared_ptr<TypeScope> scope,
                             IRGenerator& v) override;
  void PrintAst(std::ofstream& out_file);
  std::string TypeCheckExpr(std::shared_ptr<TypeScope> scope,
                            VisitorTypeCheck& v) override;

private:
  std::string op_;
  std::unique_ptr<Expr> left_;
  std::unique_ptr<Expr> right_;

  friend InterpreterBase;
  friend IRGenerator;
  friend TypeChecker;
};

class VisitorInterpret {
public:
  virtual void Interpret(Root& ref, std::shared_ptr<TypeScope> scope) const = 0;
  virtual int Interpret(Number& ref,
                        std::shared_ptr<TypeScope> scope) const = 0;
  virtual bool Interpret(Boolean& ref,
                         std::shared_ptr<TypeScope> scope) const = 0;
  virtual void Interpret(Print& ref,
                         std::shared_ptr<TypeScope> scope) const = 0;
  virtual void Interpret(Condition& ref,
                         std::shared_ptr<TypeScope> scope) const = 0;
  virtual void Interpret(Declare& ref,
                         std::shared_ptr<TypeScope> scope) const = 0;
  virtual void Interpret(Assignment& ref,
                         std::shared_ptr<TypeScope> scope) const = 0;
  virtual int Interpret(Variable& ref,
                        std::shared_ptr<TypeScope> scope) const = 0;
  virtual int Interpret(BinOp& ref, std::shared_ptr<TypeScope> scope) const = 0;

  virtual ~VisitorInterpret() = default;
};

class VisitorTypeCheck {
public:
  virtual void TypeCheck(Root& ref, std::shared_ptr<TypeScope> scope) = 0;
  virtual std::string TypeCheck(Number& ref,
                                std::shared_ptr<TypeScope> scope) = 0;
  virtual std::string TypeCheck(Boolean& ref,
                                std::shared_ptr<TypeScope> scope) = 0;
  virtual void TypeCheck(Print& ref, std::shared_ptr<TypeScope> scope) = 0;
  virtual void TypeCheck(Condition& ref, std::shared_ptr<TypeScope> scope) = 0;
  virtual void TypeCheck(Declare& ref, std::shared_ptr<TypeScope> scope) = 0;
  virtual void TypeCheck(Assignment& ref, std::shared_ptr<TypeScope> scope) = 0;
  virtual std::string TypeCheck(Variable& ref,
                                std::shared_ptr<TypeScope> scope) = 0;
  virtual std::string TypeCheck(BinOp& ref,
                                std::shared_ptr<TypeScope> scope) = 0;
  virtual ~VisitorTypeCheck() = default;
};

class TypeChecker : public VisitorTypeCheck {
public:
  void TypeCheck(Root& root) {
    auto scope = std::make_shared<TypeScope>();
    root.TypeCheckStmt(scope, *this);
  }
  void TypeCheck(Root& ref, std::shared_ptr<TypeScope> scope) override {
    ref.head->TypeCheckStmt(scope, *this);
    if (ref.next)
      ref.next->TypeCheckStmt(scope, *this);
  }
  std::string TypeCheck(Number& ref,
                        std::shared_ptr<TypeScope> scope) override {
    return "int";
  }
  std::string TypeCheck(Boolean& ref,
                        std::shared_ptr<TypeScope> scope) override {
    return "bool";
  }
  void TypeCheck(Print& ref, std::shared_ptr<TypeScope> scope) override {
    std::string expr_type = ref.expr_->TypeCheckExpr(scope, *this);
    if (expr_type != "int") {
      std::cerr << "Error: print expects int, got " << expr_type << std::endl;
    }
  }
  void TypeCheck(Condition& ref, std::shared_ptr<TypeScope> scope) override {
    std::string cond_type = ref.condition_->TypeCheckExpr(scope, *this);
    if (cond_type != "int") {
      std::cerr << "Error: condition expects int, got " << cond_type
                << std::endl;
      return;
    }
    if (cond_type != "bool") {
      std::cerr << "Error: condition expects bool, got " << cond_type
                << std::endl;
      return;
    }
    auto then_scope = std::make_shared<TypeScope>(scope);
    ref.then_stmt->TypeCheckStmt(then_scope, *this);
    if (ref.else_stmt) {
      auto else_scope = std::make_shared<TypeScope>(scope);
      ref.else_stmt->TypeCheckStmt(else_scope, *this);
    }
  }

  void TypeCheck(Declare& ref, std::shared_ptr<TypeScope> scope) override {
    if (scope->HasVar(ref.name_)) {
      std::cerr << "Error: variable " << ref.name_
                << " already declared in this scope" << std::endl;
    } else {
      if (ref.type_ == "int") {
        scope->SetVar(ref.name_, "int");
      } else {
        scope->SetVar(ref.name_, "bool");
      }
    }
  }
  void TypeCheck(Assignment& ref, std::shared_ptr<TypeScope> scope) override {
    std::string var_type = scope->GetVarType(ref.name_);
    if (var_type.empty()) {
      std::cerr << "Error: variable " << ref.name_ << " not declared"
                << std::endl;
    } else {
      std::string expr_type = ref.expr_->TypeCheckExpr(scope, *this);
      if (expr_type != var_type) {
        std::cerr << "Error: type mismatch in assignment, expected " << var_type
                  << ", got " << expr_type << std::endl;
      }
    }
  }
  std::string TypeCheck(Variable& ref,
                        std::shared_ptr<TypeScope> scope) override {
    std::string type = scope->GetVarType(ref.name_);
    if (type.empty()) {
      std::cerr << "Error: variable " << ref.name_ << " not declared"
                << std::endl;
      return "";
    }
    return type;
  }
  std::string TypeCheck(BinOp& ref, std::shared_ptr<TypeScope> scope) override {
    std::string left_type = ref.left_->TypeCheckExpr(scope, *this);
    std::string right_type = ref.right_->TypeCheckExpr(scope, *this);
    if (ref.op_ == "+" || ref.op_ == "-" || ref.op_ == "*" || ref.op_ == "/" ||
        ref.op_ == "==") {
      if (left_type == "int" && right_type == "int") {
        return "int";
      }
      std::cerr << "Error: operation expects int operands" << std::endl;
      return "";
    }
    return "";
  }
};
