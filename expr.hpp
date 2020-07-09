#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "type.hpp"

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

struct visitor {
    virtual void visit(const func_expr&) = 0;
    virtual void visit(const struct_expr&) = 0;
    virtual void visit(const real_expr&) = 0;
    virtual void visit(const var_expr&) = 0;
    virtual void visit(const let_expr&) = 0;
    virtual void visit(const add_expr&) = 0;
    virtual void visit(const sub_expr&) = 0;
    virtual void visit(const mul_expr&) = 0;
    virtual void visit(const div_expr&) = 0;
    virtual void visit(const access_expr&) = 0;
    virtual void visit(const create_expr&) = 0;
    virtual void visit(const apply_expr&) = 0;
};

struct expression {
    virtual void accept(visitor&) const {};
};

using expr = std::shared_ptr<expression>;

struct func_expr : expression {
    std::string name_;
    std::vector <std::string> args_;
    expr body_;
    type type_;

    func_expr(type ret, std::string name, std::vector <std::pair<std::string, type>> args, expr body)
            : name_(name), body_(body) {
        std::vector <type> types;
        for (auto a: args) {
            args_.push_back(a.first);
            types.push_back(a.second);
        }
        type_ = std::make_shared<func_type>(std::move(types), ret);
    }

    type signature() {
        return type_;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct struct_expr : expression {
    std::string name_;
    std::vector <std::string> fields_;
    type type_;

    struct_expr(std::string name, std::vector <std::pair<std::string, type>> fields) : name_(name) {
        std::vector <type> types;
        for (auto a: fields) {
            fields_.push_back(a.first);
            types.push_back(a.second);
        }
        type_ = std::make_shared<struct_type>(name_, std::move(types));
    }

    type signature() {
        return type_;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct real_expr : expression {
    double val_;

    real_expr(double val) : val_(val) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct var_expr : expression {
    std::string name_;

    var_expr(std::string name) : name_(name) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct let_expr : expression {
    std::string var_;
    expr val_;
    expr body_;

    let_expr(std::string var, expr val, expr body) : var_(var), val_(val), body_(body) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct add_expr : expression {
    expr lhs_;
    expr rhs_;

    add_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct sub_expr : expression {
    expr lhs_;
    expr rhs_;

    sub_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
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
};

struct access_expr : expression {
    expr struct_;
    std::string field_;

    access_expr(expr str, std::string field) : struct_(str), field_(field) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct create_expr : expression {
    expr struct_;
    std::vector <expr> fields_;

    create_expr(expr str, std::vector <expr> fields) : struct_(str), fields_(fields) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct apply_expr : expression {
    expr func_;
    std::vector <expr> args_;

    apply_expr(expr func, std::vector <expr> args) : func_(func), args_(args) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct print : visitor {
    std::ostream& out_;
    int indent_;

    print(std::ostream& out, int indent = 0) : indent_(indent), out_(out) {}

    virtual void visit(const func_expr& e) override {
        out_ << "let_f (" << e.name_ << "(";

//        auto t = static_cast<const func_type&>(*e.type_);
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            out_ << "(" << e.args_[i];// << ":" << t.args_[i]->id() << ")";
        }
        out_ << ")\n";
        e.body_->accept(*this);
    }

    virtual void visit(const struct_expr& e) override {
        out_ << "let_s (" << e.name_ << "(";

//        auto t = static_cast<const struct_type&>(*e.type_);
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            out_ << e.fields_[i];// << ":" << t.fields_[i]->id();
        }
        out_ << "))\n";
    }

    virtual void visit(const real_expr& e) override {
        out_ << e.val_;
    }

    virtual void visit(const var_expr& e) override {
    }

    virtual void visit(const let_expr& e) override {
    }

    virtual void visit(const add_expr& e) override {
    }

    virtual void visit(const sub_expr& e) override {
    }

    virtual void visit(const mul_expr& e) override {
    }

    virtual void visit(const div_expr& e) override {
    }

    virtual void visit(const access_expr& e) override {
    }

    virtual void visit(const create_expr& e) override {
    }

    virtual void visit(const apply_expr& e) override {
    }

};

