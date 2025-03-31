#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>

class InterpreterBase;
class VisitorInterpret;

class Scope {
public:
  Scope(std::shared_ptr<Scope> parent = nullptr);
  void SetVar(const std::string& name, int value);
  int GetVar(const std::string& name);
  void PrintAst(std::ofstream& out_file);

private:
  std::unordered_map<std::string, int> variables;
  std::shared_ptr<Scope> parent_;
};

inline std::shared_ptr<Scope> globalScope = std::make_shared<Scope>();

class Expr {
public:
  virtual int InterpretExpr(std::shared_ptr<Scope> scope,
                            const VisitorInterpret& v) = 0;
  virtual void PrintAst(std::ofstream& out_file) = 0;
  virtual ~Expr() = default;
};

class Stmt {
public:
  virtual void InterpretStmt(std::shared_ptr<Scope> scope,
                             const VisitorInterpret& v) = 0;
  virtual void PrintAst(std::ofstream& out_file) = 0;
  virtual ~Stmt() = default;
};

class Root : public Stmt {
public:
  Root(Stmt* head);
  Root(Stmt* stmt, Root* next);
  void InterpretStmt(std::shared_ptr<Scope> scope,
                     const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

  ~Root();

private:
  Stmt* head;
  Root* next;

  friend InterpreterBase;
};

class Number : public Expr {
public:
  Number(int value);
  int InterpretExpr(std::shared_ptr<Scope> scope,
                    const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  int value;

  friend InterpreterBase;
};

class Print : public Stmt {
public:
  Print(std::unique_ptr<Expr> expr);
  void InterpretStmt(std::shared_ptr<Scope> scope,
                     const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::unique_ptr<Expr> expr_;

  friend InterpreterBase;
};

class Condition : public Stmt {
public:
  Condition(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_stmt,
            std::unique_ptr<Stmt> else_stmt);
  void InterpretStmt(std::shared_ptr<Scope> scope,
                     const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> then_stmt;
  std::unique_ptr<Stmt> else_stmt;

  friend InterpreterBase;
};

class Declare : public Stmt {
public:
  Declare(const std::string& name);
  void InterpretStmt(std::shared_ptr<Scope> scope,
                     const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::string name_;
  int value_;

  friend InterpreterBase;
};

class Assignment : public Stmt {
public:
  Assignment(const std::string& name, std::unique_ptr<Expr> expr);
  void InterpretStmt(std::shared_ptr<Scope> scope,
                     const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::string name_;
  std::unique_ptr<Expr> expr_;

  friend InterpreterBase;
};

class Variable : public Expr {
public:
  Variable(const std::string& name);
  int InterpretExpr(std::shared_ptr<Scope> scope,
                    const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::string name_;

  friend InterpreterBase;
};

class BinOp : public Expr {
public:
  BinOp(const std::string& op, std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right);
  int InterpretExpr(std::shared_ptr<Scope> scope,
                    const VisitorInterpret& v) override;
  void PrintAst(std::ofstream& out_file);

private:
  std::string op_;
  std::unique_ptr<Expr> left_;
  std::unique_ptr<Expr> right_;

  friend InterpreterBase;
};

class VisitorInterpret {
public:
  virtual void Interpret(Root& ref, std::shared_ptr<Scope> scope) const = 0;
  virtual int Interpret(Number& ref, std::shared_ptr<Scope> scope) const = 0;
  virtual void Interpret(Print& ref, std::shared_ptr<Scope> scope) const = 0;
  virtual void Interpret(Condition& ref,
                         std::shared_ptr<Scope> scope) const = 0;
  virtual void Interpret(Declare& ref, std::shared_ptr<Scope> scope) const = 0;
  virtual void Interpret(Assignment& ref,
                         std::shared_ptr<Scope> scope) const = 0;
  virtual int Interpret(Variable& ref, std::shared_ptr<Scope> scope) const = 0;
  virtual int Interpret(BinOp& ref, std::shared_ptr<Scope> scope) const = 0;

  virtual ~VisitorInterpret() = default;
};

class InterpreterBase : public VisitorInterpret {
public:
  void Interpret(Root& ref, std::shared_ptr<Scope> scope) const override;
  int Interpret(Number& ref, std::shared_ptr<Scope> scope) const override;
  void Interpret(Print& ref, std::shared_ptr<Scope> scope) const override;
  void Interpret(Condition& ref, std::shared_ptr<Scope> scope) const override;
  void Interpret(Declare& ref, std::shared_ptr<Scope> scope) const override;
  void Interpret(Assignment& ref, std::shared_ptr<Scope> scope) const override;
  int Interpret(Variable& ref, std::shared_ptr<Scope> scope) const override;
  int Interpret(BinOp& ref, std::shared_ptr<Scope> scope) const override;
};
