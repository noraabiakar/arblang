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

    add_expr(expr lhs, expr rhs) : lhs_(lhs), rhs_(rhs) {
        if (!(lhs->signature()->is_real()) ||
            !(rhs->signature()->is_real()) ) {
            throw std::runtime_error("invalid argument types to add");
        }
    }

    virtual type signature() const override {
        return std::make_shared<real_type>();
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    add_expr* is_add() override {return this;}
};

struct access_rep : ir_expression {
    ir_ptr struct_;
    std::string field_;
    unsigned index_;

    access_rep(ir_ptr str, std::string field) : struct_(str), field_(field) {
        if (!struct_->is_struct()) {
            throw std::runtime_error("cannot access field of a non-struct type");
        }

        auto& flds = struct_->is_struct()->fields_;
        auto it = std::find_if(flds.begin(), flds.end(), [&](auto& f){return f == field_;});

        if (it == flds.end()) {
            throw std::runtime_error("field " + field_ + " does not exist in " + struct_->is_struct()->name_);
        }
        index_ = it - flds.begin();

    }

    virtual type signature() const override {
        return struct_->is_struct()->signature()->is_struct()->fields_[index_];
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    access_rep* is_access() override {return this;}
};

struct create_rep : ir_expression {
    ir_ptr struct_;
    std::vector<expr> fields_;

    create_rep(ir_ptr str, std::vector<expr> fields) : struct_(str), fields_(fields) {
        if (!struct_->is_struct()) {
            throw std::runtime_error("cannot create create_rep out of a non-struct type");
        }
    }

    virtual type signature() const override {
        return struct_->is_struct()->signature();
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

   create_rep* is_create() override {return this;}
};

struct apply_rep : ir_expression {
    ir_ptr func_;
    std::vector<expr> args_;

    apply_rep(ir_ptr func, std::vector<expr> args) : func_(func), args_(args) {
        if (!func_->is_func()){
            throw std::runtime_error("cannot apply a non-function type");
        }
    }

    virtual type signature() const override {
        return func_->is_func()->signature()->is_func()->ret_;
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    apply_rep* is_apply() override {return this;}
};

struct halt_rep : ir_expression {
    virtual type signature() const override {
        return std::make_shared<null_type>();
    }
    virtual void accept(visitor& v) const override { v.visit(*this); };
};

struct nested_rep: ir_expression {
    std::vector<type> scoped_types_;
    ir_ptr statement_;

    nested_rep(block_expr* b): scoped_types_(b->scoped_types_) {
        for (unsigned i = 0; i < b->statements_.size(); ++i) {
            auto& s0_scope = b->statements_[i]->is_func() ? b->statements_[i]->is_func()->scope_ : b->statements_[i]->is_struct()->scope_;
            if (i == b->statements_.size() -1) {
                s0_scope = std::make_shared<halt_rep>();
            } else {
                s0_scope = b->statements_[i+1];
            };
        }

        statement_ = b->statements_.front();
    }

    virtual type signature() const override {
        return std::make_shared<null_type>();
    }

    virtual void accept(visitor& v) const override { v.visit(*this); };

    nested_rep* is_nested() override {return this;}
};

struct print : visitor {
    std::ostream& out_;

    print(std::ostream& out) : out_(out) {}

    virtual void visit(const func_rep& e) override {
        out_ << "(let_f (" << e.name_ << " (";

        auto t = e.type_->is_func();
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            out_ << e.args_[i] << ":" << t->args_[i]->id() << " ";
        }
        out_ << ") ";
        e.body_->accept(*this);
        out_ << "))\n";
    }

    virtual void visit(const struct_rep& e) override {
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

    virtual void visit(const varref_rep& e) override {
        out_ << e.name_;
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

    virtual void visit(const access_rep& e) override {
        out_ << e.struct_->is_struct()->name_ << "." << e.field_;
    }

    virtual void visit(const create_rep& e) override {
        auto s = e.struct_->is_struct();
        out_ << "(create " << s->name_ << "(";
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

    virtual void visit(const ir_expression& e) override {}
};

struct print_ir : visitor {
    std::ostream& out_;
    int indent_;

    print_ir(std::ostream& out) : out_(out), indent_(0) {}

    virtual void visit(const func_rep& e) override {
        out_ << "(let_f (" << e.name_ << " (";

        auto t = e.type_->is_func();
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            out_ << e.args_[i] << ":" << t->args_[i]->id() << " ";
        }
        out_ << ") ";
        e.body_->accept(*this);
        out_ << ")\n";

        out_ << move(indent_) << "in";
        indent_+=4;

        e.scope_->accept(*this);

        indent_-=4;
        out_ << "\n" << move(indent_) << ")";

    }

    virtual void visit(const struct_rep& e) override {
        out_ << "(let_s (" << e.name_ << " (";

        auto t = e.type_->is_struct();
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            out_ << e.fields_[i] << ":" << t->fields_[i]->id() << " ";
        }
        out_ << "))\n";

        out_ << move(indent_) << "in";
        indent_+=4;

        e.scope_->accept(*this);

        indent_-=4;
        out_ << "\n" << move(indent_) << ")";
    }

    virtual void visit(const real_expr& e) override {
        out_ << e.val_;
    }

    virtual void visit(const varref_rep& e) override {
        out_ << e.name_;
    }

    virtual void visit(const let_rep& e) override {
        out_ << "(let (" << e.var_ << " (";
        e.val_->accept(*this);
        out_ << "))\n";

        indent_+=4;
        out_ << move(indent_) << "in";

        e.scope_->accept(*this);

        out_ << "\n" << move(indent_) << ")";
        indent_-=4;
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

    virtual void visit(const access_rep& e) override {
        out_ << e.struct_->is_struct()->name_ << "." << e.field_;
    }

    virtual void visit(const create_rep& e) override {
        auto s = e.struct_->is_struct();
        out_ << "(create " << s->name_ << "(";
        for (auto& a: e.fields_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << "))";
    }

    virtual void visit(const nested_rep& e) override {
        e.statement_->accept(*this);
    }

    virtual void visit(const halt_rep& e) override {
        out_ << "()";
    }

    virtual void visit(const ir_expression& e) override {}

private:
    std::string move(int x) {
        return std::string(x, ' ');
    }
};

} //namespace ir