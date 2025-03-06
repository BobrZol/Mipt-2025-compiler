#include <iostream>
#include <memory>
#include <unordered_map>

inline std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

class Scope {
public:
  Scope(std::shared_ptr<Scope> parent = nullptr) : parent_(parent) {}
  void SetVar(const std::string& name, int value) { variables[name] = value; }
  int GetVar(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end())
      return it->second;
    if (parent_)
      return parent_->GetVar(name);
    std::cerr << "Error: Variable " << name << " not found" << std::endl;
    exit(1);
  }

private:
  std::unordered_map<std::string, int> variables;
  std::shared_ptr<Scope> parent_;
};

class Expr {
public:
  virtual int InterpretExpr(std::shared_ptr<Scope> scope) = 0;
  virtual ~Expr() = default;
};

class Stmt {
public:
  virtual void InterpretStmt(std::shared_ptr<Scope> scope) = 0;
  virtual ~Stmt() = default;
};

class Root : public Stmt {
public:
  Root(Stmt* head) : head(head) {}
  Root(Stmt* stmt, Root* next) : head(stmt), next(next) {}
  void InterpretStmt(std::shared_ptr<Scope> scope) override {
    // std::cout << "Root" << '\n';
    head->InterpretStmt(scope);
    if (next != nullptr)
      next->InterpretStmt(scope);
    delete head;
  }

private:
  Stmt* head;
  Root* next;
};

class Number : public Expr {
public:
  Number(int value) : value(value) {}
  int InterpretExpr(std::shared_ptr<Scope> scope) override { return value; }

private:
  int value;
};

class Print : public Stmt {
public:
  Print(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}
  void InterpretStmt(std::shared_ptr<Scope> scope) override {
    // std::cout << "Print" << '\n';
    std::cout << expr_->InterpretExpr(scope) << '\n';
  }

private:
  std::unique_ptr<Expr> expr_;
};

class Condition : public Stmt {
public:
  Condition(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_stmt,
            std::unique_ptr<Stmt> else_stmt)
      : condition_(std::move(condition)), then_stmt(std::move(then_stmt)),
        else_stmt(std::move(else_stmt)) {}
  void InterpretStmt(std::shared_ptr<Scope> scope) override {
    if (condition_->InterpretExpr(scope)) {
      then_stmt->InterpretStmt(std::make_shared<Scope>(scope));
    } else {
      else_stmt->InterpretStmt(std::make_shared<Scope>(scope));
    }
  }

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> then_stmt;
  std::unique_ptr<Stmt> else_stmt;
};

class Declare : public Stmt {
public:
  Declare(const std::string& name) : name_(name), value_(0) {}
  void InterpretStmt(std::shared_ptr<Scope> scope) override {
    // std::cout << "Declare" << '\n';
    scope->SetVar(name_, value_);
  }

private:
  std::string name_;
  int value_;
};

class Assignment : public Stmt {
public:
  Assignment(const std::string& name, std::unique_ptr<Expr> expr)
      : name_(name), expr_(std::move(expr)) {}
  void InterpretStmt(std::shared_ptr<Scope> scope) override {
    scope->SetVar(name_, expr_->InterpretExpr(scope));
  }

private:
  std::string name_;
  std::unique_ptr<Expr> expr_;
};

class Variable : public Expr {
public:
  Variable(const std::string& name) : name_(name) {}
  int InterpretExpr(std::shared_ptr<Scope> scope) override {
    return scope->GetVar(name_);
  }

private:
  std::string name_;
};

class BinOp : public Expr {
public:
  BinOp(const std::string& op, std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right)
      : op_(op), left_(std::move(left)), right_(std::move(right)) {}
  int InterpretExpr(std::shared_ptr<Scope> scope) override {
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

private:
  std::string op_;
  std::unique_ptr<Expr> left_;
  std::unique_ptr<Expr> right_;
};
