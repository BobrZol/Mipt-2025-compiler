#include "Ast.hpp"

BinOp::BinOp(const std::string& op, std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {}

int BinOp::InterpretExpr(std::shared_ptr<Scope> scope,
                         const VisitorInterpret& v) {
  return v.Interpret(*this, scope);
}

void BinOp::PrintAst(std::ofstream& out_file) {
  out_file << "BinOp: " << op_ << '\n';
  left_->PrintAst(out_file);
  right_->PrintAst(out_file);
}

Condition::Condition(std::unique_ptr<Expr> condition,
                     std::unique_ptr<Stmt> then_stmt,
                     std::unique_ptr<Stmt> else_stmt)
    : condition_(std::move(condition)), then_stmt(std::move(then_stmt)),
      else_stmt(std::move(else_stmt)) {}

void Condition::InterpretStmt(std::shared_ptr<Scope> scope,
                              const VisitorInterpret& v) {
  v.Interpret(*this, scope);
}

void Condition::PrintAst(std::ofstream& out_file) {
  out_file << "if (\n";
  condition_->PrintAst(out_file);
  out_file << ") then\n";
  then_stmt->PrintAst(out_file);
  out_file << "else\n";
  else_stmt->PrintAst(out_file);
}

Declare::Declare(const std::string& name) : name_(name), value_(0) {}

void Declare::InterpretStmt(std::shared_ptr<Scope> scope,
                            const VisitorInterpret& v) {
  v.Interpret(*this, scope);
}

void Declare::PrintAst(std::ofstream& out_file) {
  out_file << "Declare: " << name_ << " = " << value_ << '\n';
}

Number::Number(int value) : value(value) {}

int Number::InterpretExpr(std::shared_ptr<Scope> scope,
                          const VisitorInterpret& v) {
  return v.Interpret(*this, scope);
}

void Number::PrintAst(std::ofstream& out_file) {
  out_file << "Number: " << value << '\n';
}

Print::Print(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

void Print::PrintAst(std::ofstream& out_file) {
  out_file << "StmtPrint\n";
  expr_->PrintAst(out_file);
}

void Print::InterpretStmt(std::shared_ptr<Scope> scope,
                          const VisitorInterpret& v) {
  v.Interpret(*this, scope);
}

Root::Root(Stmt* head) : head(head), next(nullptr) {}

Root::Root(Stmt* stmt, Root* next) : head(stmt), next(next) {}

void Root::InterpretStmt(std::shared_ptr<Scope> scope,
                         const VisitorInterpret& v) {
  v.Interpret(*this, scope);
}
Root::~Root() {
  if (next != nullptr)
    next->~Root();

  delete head;
}

void Root::PrintAst(std::ofstream& out_file) {
  head->PrintAst(out_file);
  if (next != nullptr)
    next->PrintAst(out_file);
}

Scope::Scope(std::shared_ptr<Scope> parent) : parent_(parent) {}

void Scope::SetVar(const std::string& name, int value) {
  variables[name] = value;
}

int Scope::GetVar(const std::string& name) {
  auto it = variables.find(name);
  if (it != variables.end())
    return it->second;
  if (parent_)
    return parent_->GetVar(name);

  std::cerr << "Error: Variable " << name << " not found" << std::endl;
  exit(1);
}
Variable::Variable(const std::string& name) : name_(name) {}

int Variable::InterpretExpr(std::shared_ptr<Scope> scope,
                            const VisitorInterpret& v) {
  return v.Interpret(*this, scope);
}

void Variable::PrintAst(std::ofstream& out_file) {
  out_file << "Variable: " << name_ << '\n';
}

Assignment::Assignment(const std::string& name, std::unique_ptr<Expr> expr)
    : name_(name), expr_(std::move(expr)) {}

void Assignment::InterpretStmt(std::shared_ptr<Scope> scope,
                               const VisitorInterpret& v) {
  v.Interpret(*this, scope);
}

void Assignment::PrintAst(std::ofstream& out_file) {
  out_file << "Assignment: " << name_ << " =\n";
  expr_->PrintAst(out_file);
}

void InterpreterBase::Interpret(Root& ref, std::shared_ptr<Scope> scope) const {
  ref.head->InterpretStmt(scope, *this);
  if (ref.next != nullptr)
    ref.next->InterpretStmt(scope, *this);
}
int InterpreterBase::Interpret(Number& ref,
                               std::shared_ptr<Scope> scope) const {
  return ref.value;
}
void InterpreterBase::Interpret(Print& ref,
                                std::shared_ptr<Scope> scope) const {
  std::cout << ref.expr_->InterpretExpr(scope, *this) << '\n';
}
void InterpreterBase::Interpret(Condition& ref,
                                std::shared_ptr<Scope> scope) const {
  if (ref.condition_->InterpretExpr(scope, *this)) {
    ref.then_stmt->InterpretStmt(std::make_shared<Scope>(scope), *this);
  } else {
    ref.else_stmt->InterpretStmt(std::make_shared<Scope>(scope), *this);
  }
}
void InterpreterBase::Interpret(Declare& ref,
                                std::shared_ptr<Scope> scope) const {
  scope->SetVar(ref.name_, ref.value_);
}
void InterpreterBase::Interpret(Assignment& ref,
                                std::shared_ptr<Scope> scope) const {
  scope->SetVar(ref.name_, ref.expr_->InterpretExpr(scope, *this));
}
int InterpreterBase::Interpret(Variable& ref,
                               std::shared_ptr<Scope> scope) const {
  return scope->GetVar(ref.name_);
}
int InterpreterBase::Interpret(BinOp& ref, std::shared_ptr<Scope> scope) const {
  int l = ref.left_->InterpretExpr(scope, *this);
  int r = ref.right_->InterpretExpr(scope, *this);

  if (ref.op_ == "+")
    return l + r;
  if (ref.op_ == "-")
    return l - r;
  if (ref.op_ == "*")
    return l * r;

  if (ref.op_ == "/") {
    if (r == 0) {
      std::cerr << "Division by zero";
      return 0;
    }
    return l / r;
  }

  if (ref.op_ == "==")
    return l == r;

  return 0;
}
