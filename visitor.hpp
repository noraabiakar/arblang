#pragma once

#include <set>

#include "core_arblang.hpp"
#include "ir_arblang.hpp"

namespace core {

struct visitor {
    virtual void visit(const expression&) = 0;
    virtual void visit(const func_expr& e) { visit((expression&) e); };
    virtual void visit(const struct_expr& e) { visit((expression&) e); };
    virtual void visit(const float_expr& e) { visit((expression&) e); };
    virtual void visit(const varref_expr& e) { visit((expression&) e); };
    virtual void visit(const vardef_expr& e) { visit((expression&) e); };
    virtual void visit(const let_expr& e) { visit((expression&) e); };
    virtual void visit(const binary_expr& e) { visit((expression&) e); };
    virtual void visit(const access_expr& e) { visit((expression&) e); };
    virtual void visit(const create_expr& e) { visit((expression&) e); };
    virtual void visit(const apply_expr& e) { visit((expression&) e); };
    virtual void visit(const block_expr& e) { visit((expression&) e); };
    virtual void visit(const halt_expr& e) { visit((expression&) e); };
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

    virtual void visit(const vardef_expr& e) override {
        out_ << e.var_ << ":" << e.type_;
    }

    virtual void visit(const varref_expr& e) override {
        out_ << e.var_;
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

struct create_ir : visitor {
    std::unordered_map<std::string, type_ptr> def_types_;
    std::unordered_map<std::string, ir::ir_ptr> scope_vars_;

    ir::ir_ptr statement_;

    create_ir() : def_types_({{"float", std::make_shared<float_type>()}}) {};

    void reset() {
        statement_ = nullptr;
        scope_vars_.clear();
    }

    virtual void visit(const func_expr& e) override {
        auto it = def_types_.find(e.ret_);
        if (it == def_types_.end()) {
            throw std::runtime_error("Function \"" + e.name_ + "\"'s return type \"" + e.ret_ + "\" is undefined");
        }
        auto ret = it->second;

        std::vector<ir::ir_ptr> args;
        for (auto& a: e.args_) {
            a->accept(*this);
            args.push_back(statement_);
        }

        e.body_->accept(*this);
        auto body = statement_;

        statement_ = std::make_shared<ir::func_rep>(e.name_, ret, args, body);
        def_types_.insert({e.name_, statement_->type()});
    }

    virtual void visit(const struct_expr& e) override {
        std::vector<ir::ir_ptr> fields;
        for (auto& a: e.fields_) {
            a->accept(*this);
            fields.push_back(statement_);
        }

        statement_ = std::make_shared<ir::struct_rep>(e.name_, fields);
        def_types_.insert({e.name_, statement_->type()});
    }

    virtual void visit(const float_expr& e) override {
        statement_ = std::make_shared<ir::float_rep>(e.val_);
    }

    virtual void visit(const vardef_expr& e) override {
        auto it = def_types_.find(e.type_);
        if (it == def_types_.end()) {
            throw std::runtime_error("Variable definiton \"" + e.var_ + "\"'s type \"" + e.type_ + "\" is undefined");
        }
        statement_ = std::make_shared<ir::vardef_rep>(e.var_, it->second);
        scope_vars_.insert({e.var_, statement_});
    }

    virtual void visit(const varref_expr& e) override {
        auto it = scope_vars_.find(e.var_);
        if (it == scope_vars_.end()) {
            throw std::runtime_error("Variable reference \"" + e.var_ + "\" is undefined");
        }
        auto def = it->second;

        statement_ = std::make_shared<ir::varref_rep>(def, def->type());
    }

    virtual void visit(const let_expr& e) override {
        e.var_->accept(*this);
        auto var = statement_;

        e.val_->accept(*this);
        auto val = statement_;

        e.body_->accept(*this);
        auto body = statement_;

        statement_ = std::make_shared<ir::let_rep>(var, val, body, body->type());
    }

    virtual void visit(const binary_expr& e) override {
        e.lhs_->accept(*this);
        auto lhs = statement_;

        e.rhs_->accept(*this);
        auto rhs = statement_;

        auto t =   lhs->type();
        auto r =   rhs->type();

        if (lhs->type()->name() != rhs->type()->name()) {
            throw std::runtime_error("Cannot perform binary operation on incompatible types");
        }
        if (!lhs->type()->is_float()) {
            throw std::runtime_error("Cannot perform binary operation on non-float types");
        }

        statement_ = std::make_shared<ir::binary_rep>(lhs, rhs, e.op_, lhs->type());
    }

    virtual void visit(const access_expr& e) override {
        auto it = scope_vars_.find(e.object_);
        if (it == scope_vars_.end()) {
            throw std::runtime_error("Variable reference \"" + e.object_ + "\" is undefined");
        }

        auto def = it->second;
        auto ref = std::make_shared<ir::varref_rep>(def, def->type());

        if (auto obj = def->type()->is_struct()) {
            unsigned i = 0;
            for (; i < obj->fields_.size(); ++i) {
                if (obj->fields_[i].name == e.field_) {
                    statement_ = std::make_shared<ir::access_rep>(ref, i, obj->fields_[i].type);
                    return;
                }
            }
            throw std::runtime_error("Object \"" + e.object_ + "\" doesnot contain fields  \"" + e.field_+ "\"");
        }
        throw std::runtime_error("Variable reference \"" + e.object_ + "\" doesn't have a struct type; cannot access member \"" + e.field_+ "\"");
    }

    virtual void visit(const create_expr& e) override {
        auto it = def_types_.find(e.struct_);
        if (it == def_types_.end()) {
            throw std::runtime_error("Cannot create object of type \"" + e.struct_ + "\"' because the type hasn't been defined");
        }
        auto strct = it->second;

        if (!strct->is_struct()) {
            throw std::runtime_error("Cannot create object of non-struct type");
        }

        std::vector<ir::ir_ptr> fields;
        const auto& struct_fields = strct->is_struct()->fields_;

        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            const auto& f = e.fields_[i];
            f->accept(*this);

            if (statement_->type() != struct_fields[i].type) {
                throw std::runtime_error("Cannot create object: incorrect type for field " + std::to_string(i));
            }
            fields.push_back(statement_);
        }
        statement_ = std::make_shared<ir::create_rep>(fields, strct);
    }

    virtual void visit(const apply_expr& e) override {
        auto it = def_types_.find(e.func_);
        if (it == def_types_.end() || !it->second->is_func()) {
            throw std::runtime_error("Cannot apply function \"" + e.func_ + "\"' because the it hasn't been defined");
        }
        auto func = it->second;

        if (!func->is_func()) {
            throw std::runtime_error("Cannot apply a non-function type");
        }

        std::vector<ir::ir_ptr> args;
        const auto& func_args = func->is_func()->args_;

        for (unsigned i = 0; i < e.args_.size(); ++i) {
            const auto& f = e.args_[i];
            f->accept(*this);

            if (statement_->type() != func_args[i].type) {
                throw std::runtime_error("Cannot apply function: incorrect type for argument " + std::to_string(i));
            }
            args.push_back(statement_);
        }
        statement_ = std::make_shared<ir::apply_rep>(args, func);
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
    virtual void visit(const vardef_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const varref_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const let_rep& e)    {visit((ir_expression&) e);};
    virtual void visit(const binary_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const access_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const create_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const apply_rep& e)  {visit((ir_expression&) e);};
    virtual void visit(const nested_rep& e) {visit((ir_expression&) e);};
    virtual void visit(const halt_rep& e)   {visit((ir_expression&) e);};
};

struct canonical : visitor {
    ir_ptr statement_;
    unsigned idx_ = 0;

    std::string unique_id() {
        return "_ll" + std::to_string(idx_++);
    }

    canonical() {}

    virtual void visit(const func_rep& e) override {
        e.body_->accept(*this);
        auto new_body = statement_;

        statement_ = std::make_shared<func_rep>(e.name_, e.args_, new_body, e.type());
    }

    virtual void visit(const struct_rep& e) override {
        statement_ = std::make_shared<struct_rep>(e);
    }

    virtual void visit(const float_rep& e) override {
        statement_ = std::make_shared<float_rep>(e);
    }

    virtual void visit(const vardef_rep& e) override {
        statement_ = std::make_shared<vardef_rep>(e);
    }

    virtual void visit(const varref_rep& e) override {
        statement_ = std::make_shared<varref_rep>(e);
    }

    virtual void visit(const let_rep& e) override {
        e.val_->accept(*this);
        auto new_val = statement_;

        e.scope_->accept(*this);
        auto new_scope = statement_;

        statement_ = std::make_shared<let_rep>(e.var_, new_val, new_scope);
    }

    virtual void visit(const binary_rep& e) override {
        if ((e.lhs_->is_varref() || e.lhs_->is_float()) &&
            (e.rhs_->is_varref() || e.rhs_->is_float())) {

            auto var = unique_id();
            auto vardef = std::make_shared<vardef_rep>(var, e.lhs_->type());
            auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
            statement_ = std::make_shared<let_rep>(vardef, std::make_shared<binary_rep>(e), e.type());
            return;
        }
        if (!(e.lhs_->is_varref() || e.lhs_->is_float()) &&
             (e.rhs_->is_varref() || e.rhs_->is_float())) {

            e.lhs_->accept(*this);
            auto lhs_stmt = statement_;

            if (!lhs_stmt->is_let()) {
                throw std::runtime_error("LHS of bin expression has wrong type");
            }

            auto lhs_var = lhs_stmt->is_let()->var_;
            auto lhs_val = lhs_stmt->is_let()->val_;

            auto var = unique_id();
            auto vardef = std::make_shared<vardef_rep>(var, lhs_var->type());
            auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
            auto bin    = std::make_shared<binary_rep>(lhs_var, e.rhs_, e.op_, e.type());
            auto let    = std::make_shared<let_rep>(vardef, bin);

            statement_ = std::make_shared<let_rep>(lhs_var, lhs_val, let, let->type());
            return;
        }
        if ((e.lhs_->is_varref() || e.lhs_->is_float()) &&
           !(e.rhs_->is_varref() || e.rhs_->is_float())) {

            e.rhs_->accept(*this);
            auto rhs_stmt = statement_;

            if (!rhs_stmt->is_let()) {
                throw std::runtime_error("LHS of bin expression has wrong type");
            }

            auto rhs_var = rhs_stmt->is_let()->var_;
            auto rhs_val = rhs_stmt->is_let()->val_;

            auto var = unique_id();
            auto vardef = std::make_shared<vardef_rep>(var, rhs_var->type());
            auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
            auto bin    = std::make_shared<binary_rep>(e.lhs_, rhs_var, e.op_, e.type());
            auto let    = std::make_shared<let_rep>(vardef, bin, e.type());

            statement_ = std::make_shared<let_rep>(rhs_var, rhs_val, let, let->type());
            return;
        }
        e.lhs_->accept(*this);
        auto lhs_stmt = statement_;

        e.rhs_->accept(*this);
        auto rhs_stmt = statement_;

        if (!lhs_stmt->is_let() || !rhs_stmt->is_let()) {
            throw std::runtime_error("LHS/RHS of bin expression has wrong type");
        }

        auto rhs_var = rhs_stmt->is_let()->var_;
        auto lhs_var = lhs_stmt->is_let()->var_;

        auto rhs_val = rhs_stmt->is_let()->val_;
        auto lhs_val = lhs_stmt->is_let()->val_;

        auto var = unique_id();
        auto vardef = std::make_shared<vardef_rep>(var, lhs_var->type());
        auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
        auto bin    = std::make_shared<binary_rep>(lhs_var, rhs_var, e.op_, e.type());
        auto let    = std::make_shared<let_rep>(vardef, bin);

        auto lhs_let = std::make_shared<let_rep>(lhs_var, lhs_val, let, let->type());
        statement_   = std::make_shared<let_rep>(rhs_var, rhs_val, lhs_let, lhs_let->type());
        return;
    }

    virtual void visit(const access_rep& e) override {
        statement_ = std::make_shared<access_rep>(e);
    }

    virtual void visit(const create_rep& e) override {
        std::vector<ir_ptr> new_fields;

        for (auto& f: e.fields_) {
            f->accept(*this);
            auto stmt = statement_;
            if ((stmt->is_varref() || stmt->is_float()) {
                new_fields.push_back(stmt)
            }

        }
    }

    virtual void visit(const ir_expression& e) override {}

};

struct print : visitor {
    std::ostream& out_;

    print(std::ostream& out) : out_(out) {}

    virtual void visit(const func_rep& e) override {
        out_ << "(";
        if (auto t = e.type_->is_func()->ret_->is_struct()) {
            out_ << t->name_;
        } else {
            out_ << "float";
        }
        out_ << " let_f (" << e.name_ << " (";

        for (auto& a: e.args_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << ") ";
        e.body_->accept(*this);
        out_ << "))\n";
    }

    virtual void visit(const struct_rep& e) override {
        out_ << "(let_s (" << e.name_ << " (";

        for (auto& f: e.fields_) {
            f->accept(*this);
            out_ << " ";
        }
        out_ << ")))\n";
    }

    virtual void visit(const float_rep& e) override {
        out_ << e.val_;
    }

    virtual void visit(const vardef_rep& e) override {
        out_ << e.name_ << ":";
        if (auto t = e.type_->is_struct()) {
            out_ << t->name_;
        } else if (auto t = e.type_->is_float()){
            out_ << "float";
        }
    }

    virtual void visit(const varref_rep& e) override {
        out_ << e.type()->name();
    }

    virtual void visit(const let_rep& e) override {
        out_ << "(let \t(";
        e.var_->accept(*this);
        out_ << " (";
        e.val_->accept(*this);
        out_ << "))\nin\t";

        e.scope_->accept(*this);

        out_ << ")\n";
    }

    virtual void visit(const binary_rep& e) override {
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

    virtual void visit(const access_rep& e) override {
        e.var_->accept(*this);
        out_ << ".at(" << e.index_ << ")";
    }

    virtual void visit(const create_rep& e) override {
        out_ << "(create ";
        if (auto t = e.type_->is_struct()) {
            out_ << t->name_ << "(";
        } else {
            out_ << "float(";
        };
        for (auto& a: e.fields_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << "))";
    }

    virtual void visit(const ir_expression& e) override {
    }

};
} //namespace ir