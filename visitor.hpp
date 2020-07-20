#pragma once

#include "core_arblang.hpp"
#include "ir_arblang.hpp"

namespace core {

struct visitor {
    virtual void visit(const expression&) = 0;
    virtual void visit(const func_expr& e) { visit((expression&) e); };
    virtual void visit(const struct_expr& e) { visit((expression&) e); };
    virtual void visit(const float_expr& e) { visit((expression&) e); };
    virtual void visit(const varref_expr& e) { visit((expression&) e); };
    virtual void visit(const let_expr& e) { visit((expression&) e); };
    virtual void visit(const binary_expr& e) { visit((expression&) e); };
    virtual void visit(const access_expr& e) { visit((expression&) e); };
    virtual void visit(const create_expr& e) { visit((expression&) e); };
    virtual void visit(const apply_expr& e) { visit((expression&) e); };
    virtual void visit(const block_expr& e) { visit((expression&) e); };
    virtual void visit(const halt_expr& e) { visit((expression&) e); };
};

struct print_core_arblang : visitor {
    std::ostream& out_;

    print_core_arblang(std::ostream& out) : out_(out) {}

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
        if (e.type_.empty()) {
            out_ << e.var_;
        } else {
            out_ << e.var_ << ":" << e.type_;
        }
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

        switch (e.op_) {
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
}

/****************************************************************************/

namespace ir{

struct visitor {
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

}