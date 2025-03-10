#include "Ast.hpp"

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

Root::Root(Stmt* head) : head(head), next(nullptr) {}

Root::Root(Stmt* stmt, Root* next) : head(stmt), next(next) {}

void Root::InterpretStmt(std::shared_ptr<Scope> scope) {
  head->InterpretStmt(scope);
  if (next != nullptr)
    next->InterpretStmt(scope);

  delete head;
}

Number::Number(int value) : value(value) {}

int Number::InterpretExpr(std::shared_ptr<Scope> scope) { return value; }

Print::Print(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

void Print::InterpretStmt(std::shared_ptr<Scope> scope) {
  std::cout << expr_->InterpretExpr(scope) << '\n';
}

Condition::Condition(std::unique_ptr<Expr> condition,
                     std::unique_ptr<Stmt> then_stmt,
                     std::unique_ptr<Stmt> else_stmt)
    : condition_(std::move(condition)), then_stmt(std::move(then_stmt)),
      else_stmt(std::move(else_stmt)) {}
void Condition::InterpretStmt(std::shared_ptr<Scope> scope) {
  if (condition_->InterpretExpr(scope)) {
    then_stmt->InterpretStmt(std::make_shared<Scope>(scope));
  } else {
    else_stmt->InterpretStmt(std::make_shared<Scope>(scope));
  }
}

Declare::Declare(const std::string& name) : name_(name), value_(0) {}

void Declare::InterpretStmt(std::shared_ptr<Scope> scope) {
  scope->SetVar(name_, value_);
}

Assignment::Assignment(const std::string& name, std::unique_ptr<Expr> expr)
    : name_(name), expr_(std::move(expr)) {}

void Assignment::InterpretStmt(std::shared_ptr<Scope> scope) {
  scope->SetVar(name_, expr_->InterpretExpr(scope));
}

Variable::Variable(const std::string& name) : name_(name) {}

int Variable::InterpretExpr(std::shared_ptr<Scope> scope) {
  return scope->GetVar(name_);
}

BinOp::BinOp(const std::string& op, std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {}

int BinOp::InterpretExpr(std::shared_ptr<Scope> scope) {
  int l = left_->InterpretExpr(scope);
  int r = right_->InterpretExpr(scope);

  if (op_ == "+")
    return l + r;
  if (op_ == "-")
    return l - r;
  if (op_ == "*")
    return l * r;

  if (op_ == "/") {
    if (r == 0) {
      std::cerr << "Division by zero";
      return 0;
    }
    return l / r;
  }

  if (op_ == "==")
    return l == r;

  return 0;
}
