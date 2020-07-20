#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "core_arblang.hpp"
#include "type.hpp"

namespace ir {

struct visitor;

struct ir_expression;
struct func_rep;
struct struct_rep;
struct float_rep;
struct varref_rep;
struct let_rep;
struct binary_rep;
struct access_rep;
struct create_rep;
struct apply_rep;
struct nested_rep;
struct halt_rep;

struct ir_expression {
    virtual void accept(visitor&) = 0;

    virtual func_rep*    is_func()     {return nullptr;}
    virtual struct_rep*  is_struct()   {return nullptr;}
    virtual float_rep*   is_float()    {return nullptr;}
    virtual varref_rep*  is_varref()   {return nullptr;}
    virtual let_rep*     is_let()      {return nullptr;}
    virtual binary_rep*  is_binary()   {return nullptr;}
    virtual access_rep*  is_access()   {return nullptr;}
    virtual create_rep*  is_create()   {return nullptr;}
    virtual apply_rep*   is_apply()    {return nullptr;}
    virtual nested_rep*  is_nested()   {return nullptr;}
    virtual halt_rep*    is_halt()     {return nullptr;}
};

using ir_ptr = std::shared_ptr<ir_expression>;
using core::operation;

struct func_rep : ir_expression {
    std::string         name_;
    std::vector<ir_ptr> args_;
    ir_ptr              body_;
    ir_ptr              scope_;     // The `in` part of let_s ... in ...
    type_ptr            type_;

    func_rep(std::string name, type_ptr ret, std::vector<ir_ptr> args, ir_ptr body, ir_ptr scope);

    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    void accept(visitor& v) override;

    func_rep* is_func() override {return this;}
};

struct struct_rep : ir_expression {
    std::string         name_;
    std::vector<ir_ptr> fields_;
    ir_ptr              scope_;     // The `in` part of let_s ... in ...
    type_ptr            type_;

    struct_rep(std::string name, std::vector<ir_ptr> fields, ir_ptr scope);


    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    void accept(visitor& v) override;

    struct_rep* is_struct() override {return this;}
};

struct float_rep : ir_expression {
    double val_;
    type_ptr type_;

    float_rep(double val) : val_(val), type_(std::make_shared<float_type>()) {}

    void accept(visitor& v) override;

    float_rep* is_float() override {return this;}
};

struct varref_rep : ir_expression {
    std::string name_;
    type_ptr type_;

    varref_rep(std::string name, type_ptr t) : name_(name), type_(t) {}

    void accept(visitor& v) override;

    varref_rep* is_varref() override {return this;}
};

struct let_rep : ir_expression {
    ir_ptr var_;
    ir_ptr val_;
    ir_ptr scope_;

    let_rep(ir_ptr var, ir_ptr val, ir_ptr scope) : var_(var), val_(val), scope_(scope) {}

    void accept(visitor& v) override;

    let_rep* is_let() override {return this;}
};

struct binary_rep : ir_expression {
    ir_ptr lhs_;
    ir_ptr rhs_;
    operation op_;

    binary_rep(ir_ptr lhs, ir_ptr rhs, operation op) : lhs_(lhs), rhs_(rhs), op_(op) {}

    void accept(visitor& v) override;

    binary_rep* is_binary() override {return this;}
};

struct access_rep : ir_expression {
    ir_ptr var_;
    ir_ptr field_;

    access_rep(ir_ptr var, ir_ptr field) : var_(var_), field_(field) {}

    void accept(visitor& v) override;

    access_rep* is_access() override {return this;}
};

struct create_rep : ir_expression {
    ir_ptr object_;
    std::vector<ir_ptr> fields_;

    create_rep(ir_ptr obj, std::vector<ir_ptr> fields) : object_(obj), fields_(fields) {}

    void accept(visitor& v) override;

   create_rep* is_create() override {return this;}
};

struct apply_rep : ir_expression {
    ir_ptr func_;
    std::vector<ir_ptr> args_;

    apply_rep(ir_ptr func, std::vector<ir_ptr> args) : func_(func), args_(args) {}

    void accept(visitor& v) override;

    apply_rep* is_apply() override {return this;}
};

struct halt_rep : ir_expression {
    void accept(visitor& v) override;
};

struct nested_rep: ir_expression {
    ir_ptr statement_;

    nested_rep(ir_ptr statement): statement_(statement) {}

    void accept(visitor& v) override;

    nested_rep* is_nested() override {return this;}
};
} //namespace ir