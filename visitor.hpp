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
        def_types_.insert({e.name_, statement_->is_func()->type_});
    }

    virtual void visit(const struct_expr& e) override {
        std::vector<ir::ir_ptr> fields;
        for (auto& a: e.fields_) {
            a->accept(*this);
            fields.push_back(statement_);
        }

        statement_ = std::make_shared<ir::struct_rep>(e.name_, fields);
        def_types_.insert({e.name_, statement_->is_struct()->type_});
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
        statement_ = std::make_shared<ir::varref_rep>(e.var_, it->second);
    }

    virtual void visit(const let_expr& e) override {
        e.var_->accept(*this);
        auto var = statement_;

        e.val_->accept(*this);
        auto val = statement_;

        e.body_->accept(*this);
        auto body = statement_;

        statement_ = std::make_shared<ir::let_rep>(var, val, body);
    }

    virtual void visit(const binary_expr& e) override {
        e.lhs_->accept(*this);
        auto lhs = statement_;

        e.rhs_->accept(*this);
        auto rhs = statement_;

        std::make_shared<ir::binary_rep>(lhs, rhs, e.op_);
    }

    virtual void visit(const access_expr& e) override {
        auto it = scope_vars_.find(e.object_);
        if (it == scope_vars_.end()) {
            throw std::runtime_error("Variable reference \"" + e.object_ + "\" is undefined");
        }

        auto def = it->second;
        auto ref = std::make_shared<ir::varref_rep>(e.object_, def);

        if (auto obj = def->is_struct()) {
            unsigned i = 0;
            for (; i < obj->fields_.size(); ++i) {
                if (obj->fields_[i]->is_vardef() && obj->fields_[i]->is_vardef()->name_ == e.field_) {
                    break;
                }
            }
            statement_ = std::make_shared<ir::access_rep>(ref, i);
        }
        throw std::runtime_error("Variable reference \"" + e.object_ + "\" doesn't have a struct type; cannot access member \"" + e.field_+ "\"");
    }

    virtual void visit(const create_expr& e) override {
        auto it = def_types_.find(e.struct_);
        if (it == def_types_.end()) {
            throw std::runtime_error("Cannot create object of type \"" + e.struct_ + "\"' because the type hasn't been defined");
        }
        auto strct = it->second;

        std::vector<ir::ir_ptr> fields;
        for (auto& a: e.fields_) {
            a->accept(*this);
            fields.push_back(statement_);
        }
        if (fields.size() != strct->is_struct()->fields_.size()) {
            throw std::runtime_error("Cannot create object of type \"" + e.struct_ + "\"' too many/too few arguments provided");
        }
        statement_ = std::make_shared<ir::create_rep>(fields, strct);
    }

    virtual void visit(const apply_expr& e) override {
        auto it = def_types_.find(e.func_);
        if (it == def_types_.end() || !it->second->is_func()) {
            throw std::runtime_error("Cannot apply function \"" + e.func_ + "\"' because the it hasn't been defined");
        }
        auto func = it->second;

        std::vector<ir::ir_ptr> args;
        for (auto& a: e.args_) {
            a->accept(*this);
            args.push_back(statement_);
        }

        if (args.size() != func->is_func()->args_.size()) {
            throw std::runtime_error("Cannot apply function \"" + e.func_ + "\"' too many/too few arguments provided");
        }

        statement_ = std::make_shared<ir::apply_rep>(args, func);
    }

    virtual void visit(const expression& e) override {}


};

/*void create_arblang_ir(block_expr e) {
    std::vector<ir::ir_ptr> statements_;
    auto creator = create_ir();

    for (auto& s: e.statements_) {
        s->accept(creator);
        statements_.push_back(creator.statement_);
        creator.reset();
    }
};*/

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