#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "core_arblang.hpp"
#include "type.hpp"
namespace ir {

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

struct ir_visitor {
    virtual void visit(const ir_expression&)  = 0;
    virtual void visit(const func_rep& e)   {visit((ir_expression&) e);};
    virtual void visit(const struct_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const float_rep& e)  {visit((ir_expression&) e);};
    virtual void visit(const varref_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const let_rep& e)    {visit((ir_expression&) e);};
    virtual void visit(const binary_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const access_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const create_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const apply_rep& e)  {visit((ir_expression&) e);};
    virtual void visit(const nested_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const halt_rep& e)   {visit((ir_expression&) e);};
};

struct ir_expression {
    virtual void accept(ir_visitor&) const {};

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
    func_type           type_;

    func_rep(std::string name, type_ptr ret, std::vector<ir_ptr> args, ir_ptr body, ir_ptr scope) : name_(name), body_(body), scope_(scope) {
        std::vector<field> typed_args;
        for (auto& a: args) {
            if (auto vr = a->is_varref()) {
                typed_args.push_back({vr->name_, vr->type_});
            } else {
                throw std::runtime_error("function argument of non-varref type");
            }
        }
        type_ = func_type(name, ret, typed_args);
    }

    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    func_rep* is_func() override {return this;}
};

struct struct_rep : ir_expression {
    std::string         name_;
    std::vector<ir_ptr> fields_;
    ir_ptr              scope_;     // The `in` part of let_s ... in ...
    struct_type         type_;

    struct_rep(std::string name, std::vector<ir_ptr> fields, ir_ptr scope) : name_(name), scope_(scope) {
        std::vector<field> typed_fields;
        for (auto& f: fields) {
            if (auto vr = f->is_varref()) {
                typed_args.push_back({vr->name_, vr->type_});
            } else {
                throw std::runtime_error("struct field of non-varref type");
            }
        }
        type_ = struct_type(name, typed_args);
    }


    void set_scope(const ir_ptr& scope) {
        scope_ = scope;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    struct_rep* is_struct() override {return this;}
};

struct float_rep : ir_expression {
    double val_;
    float_type type_;

    real_expr(double val) : val_(val), type_() {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    real_expr* is_real() override {return this;}
};

struct varref_rep : ir_expression {
    std::string name_;
    type_ptr type_;

    varref_rep(std::string name, type_ptr t) : name_(name), type_(t) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    varref_rep* is_var() override {return this;}
};

struct let_rep : ir_expression {
    ir_ptr var_;
    ir_ptr val_;
    ir_ptr scope_;

    let_rep(ir_ptr var, ir_ptr val, ir_ptr scope) : var_(var), val_(val), scope_(scope) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    let_rep* is_let() override {return this;}
};

struct binary_expr : ir_expression {
    ir_ptr lhs_;
    ir_ptr rhs_;
    operation op_;

    binary_expr(ir_ptr lhs, ir_ptr rhs, operation op) : lhs_(lhs), rhs_(rhs), op_(op) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    add_expr* is_binary() override {return this;}
};

struct access_rep : ir_expression {
    ir_ptr var_;
    ir_ptr field_;

    access_rep(ir_ptr var, ir_ptr field) : var_(var_), field_(field) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    access_rep* is_access() override {return this;}
};

struct create_rep : ir_expression {
    ir_ptr object_;
    std::vector<ir_ptr> fields_;

    create_rep(ir_ptr obj, std::vector<ir_ptr> fields) : object_(obj), fields_(fields) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

   create_rep* is_create() override {return this;}
};

struct apply_rep : ir_expression {
    ir_ptr func_;
    std::vector<ir_ptr> args_;

    apply_rep(ir_ptr func, std::vector<ir_ptr> args) : func_(func), args_(args) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    apply_rep* is_apply() override {return this;}
};

struct halt_rep : ir_expression {
    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct nested_rep: ir_expression {
    ir_ptr statement_;

    nested_rep(ir_ptr statement): statement_(statement) {}

    virtual void accept(visitor& v) const override { v.visit(*this); };

    nested_rep* is_nested() override {return this;}
};

struct create_ir : visitor {
    const std::unordered_map<std::string, type_ptr>& def_types
    std::vector<ir_ptr> statements_;

    virtual void visit(const func_rep& e) override {}

    virtual void visit(const struct_rep& e) override {}

    virtual void visit(const real_expr& e) override {}

    virtual void visit(const varref_rep& e) override {}

    virtual void visit(const add_expr& e) override {}

    virtual void visit(const sub_expr& e) override {}

    virtual void visit(const mul_expr& e) override {}

    virtual void visit(const div_expr& e) override {}

    virtual void visit(const access_rep& e) override {}

    virtual void visit(const create_rep& e) override {}

    virtual void visit(const block_expr& e) override {}

    virtual void visit(const ir_expression& e) override {}
};

} //namespace ir