#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//Desugared language
namespace core {

struct visitor;

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
    virtual void accept(visitor&) = 0;

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

    void accept(visitor& v) override;

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

    void accept(visitor& v) override;

    struct_expr* is_struct() override {return this;}
};

struct float_expr : expression {
    double val_;

    float_expr(double val) : val_(val) {}

    void accept(visitor& v) override;

    float_expr* is_float() override {return this;}
};

struct varref_expr : expression {
    std::string var_;
    std::string type_;

    varref_expr(std::string var) : var_(var) {}
    varref_expr(std::string var, std::string type) : var_(var), type_(type) {}

    void accept(visitor& v) override;

    varref_expr* is_varref() override {return this;}
};

struct let_expr : expression {
    expr_ptr var_;
    expr_ptr val_;
    expr_ptr body_;

    let_expr(typed_var var, expr_ptr val, expr_ptr body) :
    var_(std::make_shared<varref_expr>(var.var, var.type)), val_(val), body_(body) {}

    void accept(visitor& v) override;

    let_expr* is_let() override {return this;}
};

struct binary_expr : expression {
    expr_ptr lhs_;
    expr_ptr rhs_;
    operation op_;

    binary_expr(expr_ptr lhs, expr_ptr rhs, operation op) : lhs_(lhs), rhs_(rhs), op_(op) {}

    void accept(visitor& v) override;

    binary_expr* is_binary() override {return this;}
};

struct access_expr : expression {
    std::string object_;
    std::string field_;

    access_expr(std::string object, std::string field) : object_(object), field_(field) {}

    void accept(visitor& v) override;

    access_expr* is_access() override {return this;}
};

struct create_expr : expression {
    std::string struct_;
    std::vector<expr_ptr> fields_;

    create_expr(std::string str, std::vector<expr_ptr> fields) : struct_(str), fields_(fields) {}

    void accept(visitor& v) override;

   create_expr* is_create() override {return this;}
};

struct apply_expr : expression {
    std::string func_;
    std::vector<expr_ptr> args_;

    apply_expr(std::string func, std::vector<expr_ptr> args) : func_(func), args_(args) {}

    void accept(visitor& v) override;

    apply_expr* is_apply() override {return this;}
};

struct block_expr : expression {
    std::vector<expr_ptr> statements_;

    block_expr(std::vector<expr_ptr> statements): statements_(statements){}

    void accept(visitor& v) override;

    block_expr* is_block() override {return this;}
};

struct halt_expr : expression {
    void accept(visitor& v) override;

    halt_expr* is_halt() override {return this;}
};
} //namespace core