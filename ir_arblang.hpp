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
struct vardef_rep;
struct varref_rep;
struct let_rep;
struct binary_rep;
struct access_rep;
struct create_rep;
struct apply_rep;

struct ir_expression {
    virtual func_rep*    is_func()     {return nullptr;}
    virtual struct_rep*  is_struct()   {return nullptr;}
    virtual float_rep*   is_float()    {return nullptr;}
    virtual vardef_rep*  is_vardef()   {return nullptr;}
    virtual varref_rep*  is_varref()   {return nullptr;}
    virtual let_rep*     is_let()      {return nullptr;}
    virtual binary_rep*  is_binary()   {return nullptr;}
    virtual access_rep*  is_access()   {return nullptr;}
    virtual create_rep*  is_create()   {return nullptr;}
    virtual apply_rep*   is_apply()    {return nullptr;}

    virtual void accept(visitor&) = 0;

    ir_expression() {};
    ir_expression(type_ptr type): type_(type) {};

    type_ptr type_;
    virtual type_ptr type() const {
        return type_;
    }
};

using ir_ptr = std::shared_ptr<ir_expression>;
using core::operation;

struct func_rep : ir_expression {
    std::string         name_;
    std::vector<ir_ptr> args_;
    ir_ptr              body_;
    ir_ptr              scope_;     // The `in` part of let_s ... in ...

    func_rep(std::string name, std::vector<ir_ptr> args, ir_ptr body, type_ptr type)
        : ir_expression(type), name_(name), args_(args), body_(body) {};

    func_rep(std::string name, type_ptr ret, std::vector<ir_ptr> args, ir_ptr body);

    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    void set_body(const ir_ptr& body) {
        body_ = body;
    }

    void accept(visitor& v) override;

    func_rep* is_func() override {return this;}
};

struct struct_rep : ir_expression {
    std::string         name_;
    std::vector<ir_ptr> fields_;
    ir_ptr              scope_;     // The `in` part of let_s ... in ...

    struct_rep(std::string name, std::vector<ir_ptr> fields);


    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    void accept(visitor& v) override;

    struct_rep* is_struct() override {return this;}
};

struct float_rep : ir_expression {
    double val_;

    float_rep(double val) : ir_expression(std::make_shared<float_type>()), val_(val) {}

    void accept(visitor& v) override;

    float_rep* is_float() override {return this;}
};

struct vardef_rep : ir_expression {
    std::string name_;

    vardef_rep(std::string name, type_ptr type) : ir_expression(type), name_(name) {}

    void accept(visitor& v) override;

    vardef_rep* is_vardef() override {return this;}
};

struct varref_rep : ir_expression {
    ir_ptr def_; // pointer to verdef

    varref_rep(ir_ptr def, type_ptr type) : ir_expression(type), def_(def) {}

    void accept(visitor& v) override;

    varref_rep* is_varref() override {return this;}
};

struct let_rep : ir_expression {
    ir_ptr var_;
    ir_ptr val_;
    ir_ptr scope_;

    let_rep(ir_ptr var, ir_ptr val) : var_(var), val_(val) {}

    let_rep(ir_ptr var, ir_ptr val, ir_ptr scope, type_ptr type) : ir_expression(type), var_(var), val_(val), scope_(scope) {}

    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    void set_type(const type_ptr type) {
        type_ = type;
    }

    void accept(visitor& v) override;

    let_rep* is_let() override {return this;}
};

struct binary_rep : ir_expression {
    ir_ptr lhs_;
    ir_ptr rhs_;
    operation op_;
    binary_rep(ir_ptr lhs, ir_ptr rhs, operation op, type_ptr type) : ir_expression(type), lhs_(lhs), rhs_(rhs), op_(op) {}

    void accept(visitor& v) override;

    binary_rep* is_binary() override {return this;}
};

struct access_rep : ir_expression {
    ir_ptr var_; //varref
    unsigned index_;

    access_rep(ir_ptr var, unsigned index, type_ptr type) : ir_expression(type), var_(var), index_(index) {}

    void accept(visitor& v) override;

    access_rep* is_access() override {return this;}
};

struct create_rep : ir_expression {
    std::vector<ir_ptr> fields_;

    create_rep(std::vector<ir_ptr> fields, type_ptr type) : ir_expression(type), fields_(fields) {}

    void accept(visitor& v) override;

   create_rep* is_create() override {return this;}
};

struct apply_rep : ir_expression {
    std::vector<ir_ptr> args_;

    apply_rep(std::vector<ir_ptr> args, type_ptr type) : ir_expression(type), args_(args) {}

    void accept(visitor& v) override;

    apply_rep* is_apply() override {return this;}
};
} //namespace ir