#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//Desugared language
namespace core {

struct expression;
struct func_expr;
struct struct_expr;
struct float_expr;
struct varref_expr;
struct let_expr;
struct binary_expr;
struct access_expr;
struct create_expr;
struct apply_expr;
struct block_expr;
struct halt_expr;

struct visitor {
    virtual void visit(const expression&)  = 0;
    virtual void visit(const func_expr& e)   {visit((expression&) e);};
    virtual void visit(const struct_expr& e) {visit((expression&) e);};
    virtual void visit(const float_expr& e)  {visit((expression&) e);};
    virtual void visit(const varref_expr& e) {visit((expression&) e);};
    virtual void visit(const let_expr& e)    {visit((expression&) e);};
    virtual void visit(const binary_expr& e) {visit((expression&) e);};
    virtual void visit(const access_expr& e) {visit((expression&) e);};
    virtual void visit(const create_expr& e) {visit((expression&) e);};
    virtual void visit(const apply_expr& e)  {visit((expression&) e);};
    virtual void visit(const block_expr& e)  {visit((expression&) e);};
    virtual void visit(const halt_expr& e)   {visit((expression&) e);};
};

struct typed_var {
    std::string var;
    std::string type;
};

enum operation {
    add,
    sub,
    mul,
    div
};

struct expression {
    virtual void accept(visitor&) const {};

    virtual func_expr*    is_func()     {return nullptr;}
    virtual struct_expr*  is_struct()   {return nullptr;}
    virtual float_expr*   is_float()    {return nullptr;}
    virtual varref_expr*  is_varref()   {return nullptr;}
    virtual let_expr*     is_let()      {return nullptr;}
    virtual binary_expr*  is_binary()   {return nullptr;}
    virtual access_expr*  is_access()   {return nullptr;}
    virtual create_expr*  is_create()   {return nullptr;}
    virtual apply_expr*   is_apply()    {return nullptr;}
    virtual block_expr*   is_block()    {return nullptr;}
    virtual halt_expr*    is_halt()     {return nullptr;}
};

using expr_ptr = std::shared_ptr<expression>;

struct func_expr : expression {
    std::string ret_;
    std::string name_;
    std::vector<expr_ptr> args_;
    expr_ptr body_;

    func_expr(std::string ret, std::string name, std::vector<typed_var> args, expr_ptr body)
    : ret_(ret), name_(name), body_(body) {
        for (const auto& t: args) {
            args_.emplace_back(std::make_shared<varref_expr>(t.var, t.type));
        }
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    func_expr* is_func() override {return this;}
};

struct struct_expr : expression {
    std::string name_;
    std::vector<expr_ptr> fields_;

    struct_expr(std::string name, std::vector<typed_var> fields) : name_(name) {
        for (const auto& t: fields) {
            fields_.emplace_back(std::make_shared<varref_expr>(t.var, t.type));
        }
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    struct_expr* is_struct() override {return this;}
};

struct float_expr : expression {
    double val_;

    float_expr(double val) : val_(val) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    float_expr* is_float() override {return this;}
};

struct varref_expr : expression {
    std::string var_;
    std::string type_;

    varref_expr(std::string var, std::string type) : var_(var), type_(type) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    varref_expr* is_varref() override {return this;}
};

struct let_expr : expression {
    expr_ptr var_;
    expr_ptr val_;
    expr_ptr body_;

    let_expr(typed_var var, expr_ptr val, expr_ptr body) :
    var_(std::make_shared<varref_expr>(var.var, var.type)), val_(val), body_(body) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    let_expr* is_let() override {return this;}
};

struct binary_expr : expression {
    expr_ptr lhs_;
    expr_ptr rhs_;
    operation op_;

    binary_expr(expr_ptr lhs, expr_ptr rhs, operation op) : lhs_(lhs), rhs_(rhs), op_(op) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    binary_expr* is_binary() override {return this;}
};

struct access_expr : expression {
    std::string object_;
    std::string field_;

    access_expr(std::string object, std::string field) : object_(object), field_(field) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    access_expr* is_access() override {return this;}
};

struct create_expr : expression {
    std::string struct_;
    std::vector<expr_ptr> fields_;

    create_expr(std::string str, std::vector<expr_ptr> fields) : struct_(str), fields_(fields) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

   create_expr* is_create() override {return this;}
};

struct apply_expr : expression {
    std::string func_;
    std::vector<expr_ptr> args_;

    apply_expr(std::string func, std::vector<expr_ptr> args) : func_(func), args_(args) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    apply_expr* is_apply() override {return this;}
};

struct block_expr : expression {
    std::vector<expr_ptr> statements_;

    block_expr(std::vector<expr_ptr> statements): statements_(statements){}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    block_expr* is_block() override {return this;}
};

struct halt_expr : expression {
    virtual void accept(visitor& v) const override { v.visit(*this); };

    halt_expr* is_halt() override {return this;}
};

struct print : visitor {
    std::ostream& out_;

    print(std::ostream& out) : out_(out) {}

    virtual void visit(const func_expr& e) override {
        out_ << "(" << e.ret_ << " let_f (" << e.name_ << " (";

        for (auto& a: e.args_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << ") ";
        e.body_->accept(*this);
        out_ << "))\n";
    }

    virtual void visit(const struct_expr& e) override {
        out_ << "(let_s (" << e.name_ << " (";

        for (auto& f: e.fields_) {
            f->accept(*this);
            out_ << " ";
        }
        out_ << ")))\n";
    }

    virtual void visit(const float_expr& e) override {
        out_ << e.val_;
    }

    virtual void visit(const varref_expr& e) override {
        out_ << e.var_ << ":" << e.type_;
    }

    virtual void visit(const let_expr& e) override {
        out_ << "(let \t(";
        e.var_->accept(*this);
        out_ << " (";
        e.val_->accept(*this);
        out_ << "))\nin\t";

        e.body_->accept(*this);

        out_ << ")\n";
    }

    virtual void visit(const binary_expr& e) override {
        out_ << "(";

        switch(e.op_) {
            case operation::add: {
                out_ << " + ";
                break;
            }
            case operation::sub: {
                out_ << " - ";
                break;
            }
            case operation::mul: {
                out_ << " * ";
                break;
            }
            case operation::div: {
                out_ << " / ";
                break;
            }
        }
        e.lhs_->accept(*this);
        out_ << " ";
        e.rhs_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const access_expr& e) override {
        out_ << e.object_ << "." << e.field_;
    }

    virtual void visit(const create_expr& e) override {
        out_ << "(create " << e.struct_ << "(";
        for (auto& a: e.fields_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << "))";
    }

    virtual void visit(const block_expr& e) override {
        for (auto& s: e.statements_) {
            s->accept(*this);
        }
    }

    virtual void visit(const expression& e) override {}
};

} //namespace core