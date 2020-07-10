#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "type.hpp"

struct pair {
    std::string name;
    type t;
};

struct func_expr;
struct struct_expr;
struct real_expr;
struct var_expr;
struct let_expr;
struct add_expr;
struct sub_expr;
struct mul_expr;
struct div_expr;
struct access_expr;
struct create_expr;
struct apply_expr;
struct block_expr;

struct visitor {
    virtual void visit(const func_expr&)   = 0;
    virtual void visit(const struct_expr&) = 0;
    virtual void visit(const real_expr&)   = 0;
    virtual void visit(const var_expr&)    = 0;
    virtual void visit(const let_expr&)    = 0;
    virtual void visit(const add_expr&)    = 0;
    virtual void visit(const sub_expr&)    = 0;
    virtual void visit(const mul_expr&)    = 0;
    virtual void visit(const div_expr&)    = 0;
    virtual void visit(const access_expr&) = 0;
    virtual void visit(const create_expr&) = 0;
    virtual void visit(const apply_expr&)  = 0;
    virtual void visit(const block_expr&)  = 0;
};

struct expression {
    virtual void accept(visitor&) const {};

    virtual func_expr*    is_func()     {return nullptr;}
    virtual struct_expr*  is_struct()   {return nullptr;}
    virtual real_expr*    is_real()     {return nullptr;}
    virtual var_expr*     is_var()      {return nullptr;}
    virtual let_expr*     is_let()      {return nullptr;}
    virtual add_expr*     is_add()      {return nullptr;}
    virtual sub_expr*     is_sub()      {return nullptr;}
    virtual mul_expr*     is_mul()      {return nullptr;}
    virtual div_expr*     is_div()      {return nullptr;}
    virtual access_expr*  is_access()   {return nullptr;}
    virtual create_expr*  is_create()   {return nullptr;}
    virtual apply_expr*   is_apply()    {return nullptr;}
    virtual block_expr*   is_block()    {return nullptr;}
};

using expr = std::shared_ptr<expression>;

struct func_expr : expression {
    std::string name_;
    std::vector<std::string> args_;
    expr body_;
    type type_;

    func_expr(std::string name, type ret, std::vector<pair> args, expr body)
            : name_(name), body_(body) {
        std::vector<type> types;
        for (auto a: args) {
            args_.push_back(a.name);
            types.push_back(a.t);
        }
        type_ = std::make_shared<func_type>(name_, std::move(types), ret);
    }

    type signature() {
        return type_;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    func_expr* is_func() override {return this;}
};

struct struct_expr : expression {
    std::string name_;
    std::vector<std::string> fields_;
    type type_;

    struct_expr(std::string name, std::vector<pair> fields) : name_(name) {
        std::vector<type> types;
        for (auto a: fields) {
            fields_.push_back(a.name);
            types.push_back(a.t);
        }
        type_ = std::make_shared<struct_type>(name_, std::move(types));
    }

    type signature() {
        return type_;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    struct_expr* is_struct() override {return this;}
};

struct real_expr : expression {
    double val_;

    real_expr(double val) : val_(val) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    real_expr* is_real() override {return this;}
};

struct var_expr : expression {
    std::string name_;

    var_expr(std::string name) : name_(name) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    var_expr* is_var() override {return this;}
};

struct let_expr : expression {
    std::string var_;
    expr val_;
    expr body_;

    let_expr(std::string var, expr val, expr body) : var_(var), val_(val), body_(body) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    let_expr* is_let() override {return this;}
};

struct add_expr : expression {
    expr lhs_;
    expr rhs_;

    add_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    add_expr* is_add() override {return this;}
};

struct sub_expr : expression {
    expr lhs_;
    expr rhs_;

    sub_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    sub_expr* is_sub() override {return this;}
};

struct mul_expr : expression {
    expr lhs_;
    expr rhs_;

    mul_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct div_expr : expression {
    expr lhs_;
    expr rhs_;

    div_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

     div_expr* is_div() override {return this;}
};

struct access_expr : expression {
    expr struct_;
    std::string field_;

    access_expr(expr str, std::string field) : struct_(str), field_(field) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    access_expr* is_access() override {return this;}
};

struct create_expr : expression {
    expr struct_;
    std::vector<expr> fields_;

    create_expr(expr str, std::vector<expr> fields) : struct_(str), fields_(fields) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

   create_expr* is_create() override {return this;}
};

struct apply_expr : expression {
    expr func_;
    std::vector<expr> args_;

    apply_expr(expr func, std::vector<expr> args) : func_(func), args_(args) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    apply_expr* is_apply() override {return this;}
};

struct block_expr : expression {
    std::vector<type> scoped_types_ = {std::make_shared<real_type>()};
    std::vector<expr> statements_;

    block_expr(std::vector<expr> statements) {
        for (auto& s: statements) {
            if (!(s->is_func() || s->is_struct())) {
                throw std::runtime_error("Block expressions can only contain struct or function definitions");
            }
            statements_.push_back(s);

            if (auto f = s->is_func()) {
                auto st_type = f->signature()->is_func();
                for (auto t: st_type->args_) {
                    auto it = std::find_if(scoped_types_.begin(), scoped_types_.end(), [&t](type& l){ return l->id() == t->id();});
                    if (it == scoped_types_.end()) {
                        throw std::runtime_error(t->id() +" undefined");
                    }
                }
                auto it = std::find_if(scoped_types_.begin(), scoped_types_.end(), [&st_type](type& l){ return l->id() == st_type->ret_->id();});
                if (it == scoped_types_.end()) {
                    throw std::runtime_error(st_type->ret_->id() +" undefined");
                }
            }

            if (auto f = s->is_struct()) {
                auto st_type = f->signature()->is_struct();
                for (auto t: st_type->fields_) {
                    auto it = std::find_if(scoped_types_.begin(), scoped_types_.end(), [&t](type& l){ return l->id() == t->id();});
                    if (it == scoped_types_.end()) {
                        throw std::runtime_error(t->id() +" undefined");
                    }
                }
            }

            auto st_type = s->is_func() ? s->is_func()->signature() : s->is_struct()->signature();
            auto it = std::find_if(scoped_types_.begin(), scoped_types_.end(), [&st_type](type& t){ return t->id() == st_type->id();});

            if (it != scoped_types_.end()) {
                throw std::runtime_error("redefinition of " + st_type->id());
            }

            scoped_types_.push_back(st_type);
        }
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    block_expr* is_block() override {return this;}
};

struct print : visitor {
    std::ostream& out_;
    int indent_;

    print(std::ostream& out, int indent = 0) : indent_(indent), out_(out) {}

    virtual void visit(const func_expr& e) override {
        out_ << "(let_f (" << e.name_ << " (";

        auto t = e.type_->is_func();
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            out_ << e.args_[i] << ":" << t->args_[i]->id() << " ";
        }
        out_ << ") ";
        e.body_->accept(*this);
        out_ << ")))\n";
    }

    virtual void visit(const struct_expr& e) override {
        out_ << "(let_s (" << e.name_ << " (";

        auto t = e.type_->is_struct();
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            out_ << e.fields_[i] << ":" << t->fields_[i]->id() << " ";
        }
        out_ << ")))\n";
    }

    virtual void visit(const real_expr& e) override {
        out_ << e.val_;
    }

    virtual void visit(const var_expr& e) override {
        out_ << e.name_;
    }

    virtual void visit(const let_expr& e) override {
        out_ << "(let (" << e.var_ << " (";
        e.val_->accept(*this);
        out_ << ")) ";
        e.body_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const add_expr& e) override {
        out_ << "( + ";
        e.lhs_->accept(*this);
        out_ << " ";
        e.rhs_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const sub_expr& e) override {
        out_ << "( - ";
        e.lhs_->accept(*this);
        out_ << " ";
        e.rhs_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const mul_expr& e) override {
        out_ << "( * ";
        e.lhs_->accept(*this);
        out_ << " ";
        e.rhs_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const div_expr& e) override {
        out_ << "( / ";
        e.lhs_->accept(*this);
        out_ << " ";
        e.rhs_->accept(*this);
        out_ << ")";
    }

    virtual void visit(const access_expr& e) override {
        out_ << e.struct_->is_struct()->name_ << "." << e.field_;
    }

    virtual void visit(const create_expr& e) override {
        auto s = e.struct_->is_struct();
        out_ << "(create " << s->name_ << "(";
        for (auto& a: e.fields_) {
            a->accept(*this);
            out_ << " ";
        }
    }

    virtual void visit(const apply_expr& e) override {
    }

    virtual void visit(const block_expr& e) override {
        for (auto& s: e.statements_) {
            s->accept(*this);
        }
    }
};

