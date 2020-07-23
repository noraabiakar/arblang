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
        out_ << "(let   (";
        e.var_->accept(*this);
        out_ << " (";
        e.val_->accept(*this);
        out_ << "))\nin  ";

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
    virtual void visit(ir_expression&)  = 0;
    virtual void visit(func_rep& e)   {visit((ir_expression&) e);};
    virtual void visit(struct_rep& e) {visit((ir_expression&) e);};
    virtual void visit(float_rep& e)  {visit((ir_expression&) e);};
    virtual void visit(vardef_rep& e) {visit((ir_expression&) e);};
    virtual void visit(varref_rep& e) {visit((ir_expression&) e);};
    virtual void visit(let_rep& e)    {visit((ir_expression&) e);};
    virtual void visit(binary_rep& e) {visit((ir_expression&) e);};
    virtual void visit(access_rep& e) {visit((ir_expression&) e);};
    virtual void visit(create_rep& e) {visit((ir_expression&) e);};
    virtual void visit(apply_rep& e)  {visit((ir_expression&) e);};
};

struct print : visitor {
    std::ostream& out_;
    unsigned indent_ = 0;

    print(std::ostream& out) : out_(out) {}

    virtual void visit(func_rep& e) override {
        out_ << "(let_f (";

        if (auto t = e.type_->is_func()->ret_->is_struct()) {
            out_ << t->name_ << " ";
        } else {
            out_ << "float ";
        }

        out_ << e.name_ << " (";

        for (auto& a: e.args_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << ")";

        indent_+=2;
        out_ << "\n" << move(indent_);

        e.body_->accept(*this);

        indent_-=2;
        out_ << move(indent_) << ")";

        if(e.scope_) {
            indent_+=2;
            out_ << "\n" << move(indent_) << "in  ";

            e.scope_->accept(*this);

            indent_-=2;
            out_ << move(indent_) << ")\n";
        } else {
            out_ << ")\n";
        }

    }

    virtual void visit(struct_rep& e) override {
        out_ << "(let_s (" << e.name_ << " (";

        for (auto& f: e.fields_) {
            f->accept(*this);
            out_ << " ";
        }
        out_ << "))";

        if(e.scope_) {
            indent_+=2;
            out_ << "\n" << move(indent_) << "in  ";

            e.scope_->accept(*this);

            indent_-=2;
            out_ << move(indent_) << ")\n";
        } else {
            out_ << ")\n";
        }
    }

    virtual void visit(float_rep& e) override {
        out_ << e.val_;
    }

    virtual void visit(vardef_rep& e) override {
        out_ << e.name_ << ":";
        if (auto t = e.type_->is_struct()) {
            out_ << t->name_;
        } else if (auto t = e.type_->is_float()){
            out_ << "float";
        }
    }

    virtual void visit(varref_rep& e) override {
        out_ << e.def_->is_vardef()->name_;
    }

    virtual void visit(let_rep& e) override {
        out_ << "(let_v (";
        e.var_->accept(*this);
        out_ << " (";
        e.val_->accept(*this);
        out_ << "))";

        if(e.scope_) {
            indent_+=2;
            out_ << "\n" << move(indent_) << "in  ";

            e.scope_->accept(*this);

            if (e.scope_->is_varref()) {
                out_ << "\n";
            }

            indent_-=2;
            out_ << move(indent_) << ")\n";
        } else {
            out_ << ")\n";
        }
    }

    virtual void visit(binary_rep& e) override {
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

    virtual void visit(access_rep& e) override {
        e.var_->accept(*this);
        out_ << ".at(" << e.index_ << ")";
    }

    virtual void visit(create_rep& e) override {
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

    virtual void visit(apply_rep& e) override {
        out_ << "(apply ";
        out_ << e.type_->is_func()->name_ << "(";
        for (auto& a: e.args_) {
            a->accept(*this);
            out_ << " ";
        }
        out_ << "))";
    }

    virtual void visit(ir_expression& e) override {
    }

private:
    std::string move(int x) {
        return std::string(x, ' ');
    }

};

struct canonical : visitor {
    std::vector<ir_ptr> new_lets;

    unsigned var_idx_ = 0;
    std::string unique_id() {
        return "_ll" + std::to_string(var_idx_++);
    }

    canonical() {}

    virtual void visit(let_rep& e) override {
        e.scope_->accept(*this);
    }

    virtual void visit(binary_rep& e) override {
        ir_ptr lhs, rhs;
        operation op = e.op_;

        if (e.lhs_->is_varref() || e.lhs_->is_float()) {
            lhs = e.lhs_;
        } else {
            e.lhs_->accept(*this);
            auto last = new_lets.back()->is_let()->var_;
            lhs = std::make_shared<varref_rep>(last, last->type());
        }
        if (e.rhs_->is_varref() || e.rhs_->is_float()) {
            rhs = e.rhs_;
        } else {
            e.rhs_->accept(*this);
            auto last = new_lets.back()->is_let()->var_;
            rhs = std::make_shared<varref_rep>(last, last->type());
        }

        auto vardef = std::make_shared<vardef_rep>(unique_id(), e.lhs_->type());
        auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
        new_lets.push_back(std::make_shared<let_rep>(vardef, std::make_shared<binary_rep>(lhs, rhs, op, lhs->type())));
    }

    virtual void visit(access_rep& e) override {
        auto vardef = std::make_shared<vardef_rep>(unique_id(), e.type());
        auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
        new_lets.push_back(std::make_shared<let_rep>(vardef, std::make_shared<access_rep>(e)));
    }

    virtual void visit(create_rep& e) override {
        std::vector<ir_ptr> fields;
        for (auto& f: e.fields_) {
            if (f->is_float() || f->is_varref()) {
                fields.push_back(f);
            } else {
                f->accept(*this);
                auto last = new_lets.back()->is_let()->var_;
                fields.push_back(std::make_shared<varref_rep>(last, last->type()));
            }
        }
        auto vardef = std::make_shared<vardef_rep>(unique_id(), e.type());
        auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
        new_lets.push_back(std::make_shared<let_rep>(vardef, std::make_shared<create_rep>(fields, e.type())));
    }

    virtual void visit(apply_rep& e) override {
        std::vector<ir_ptr> args;
        for (auto& a: e.args_) {
            if (a->is_float() || a->is_varref()) {
                args.push_back(a);
            } else {
                a->accept(*this);
                auto last = new_lets.back()->is_let()->var_;
                args.push_back(std::make_shared<varref_rep>(last, last->type()));
            }
        }
        auto vardef = std::make_shared<vardef_rep>(unique_id(), e.type());
        auto varref = std::make_shared<varref_rep>(vardef, vardef->type());
        new_lets.push_back(std::make_shared<let_rep>(vardef, std::make_shared<apply_rep>(args, e.type())));
    }

    virtual void visit(ir_expression& e) override {}

};

struct validate : visitor {
    virtual void visit(func_rep& e) override {
        if (!e.body_) {
            throw std::runtime_error("Function " + e.name_ + " has no body");
        }
        for(auto a: e.args_) {
            if (!a->is_vardef()) {
                throw std::runtime_error("Function " + e.name_ + " has an invalid argument");
            }
        }
        if (!e.type()) {
            throw std::runtime_error("Function " + e.name_ + " has no type");
        }
        if (e.type()->name() != e.name_) {
            throw std::runtime_error("Mismatch between function " + e.name_ + " name and it's type's name");
        }
        if (!e.type()->is_func()) {
            throw std::runtime_error("Function " + e.name_ + " has non-function type");
        }
        if (!e.type()->is_func()->ret_) {
            throw std::runtime_error("Function " + e.name_ + " has no return type");
        }
        if (e.type()->is_func()->args_.size() != e.args_.size()) {
            throw std::runtime_error("Mismatch between function " + e.name_ + "'s type and it's arguments");
        }

        e.body_->accept(*this);
        if (e.scope_) {
            e.scope_->accept(*this);
        }

        for (auto a:e.args_) {
            a->accept(*this);
        }

        for (unsigned i = 0; i < e.args_.size(); ++i) {
            auto t0 = e.args_[i]->type();
            auto t1 = e.type()->is_func()->args_[i].type;

            auto n0 = e.args_[i]->is_vardef()->name_;
            auto n1 = e.type()->is_func()->args_[i].name;

            if (t0 != t1 || n0 != n1) {
                throw std::runtime_error("Mismatch between function " + e.name_ + "'s type and it's arguments");
            }
        }
    }

    virtual void visit(struct_rep& e) override {
        for(auto a: e.fields_) {
            if (!a->is_vardef()) {
                throw std::runtime_error("Struct " + e.name_ + " has an invalid field");
            }
        }
        if (!e.type()) {
            throw std::runtime_error("Struct " + e.name_ + " has no type");
        }
        if (e.type()->name() != e.name_) {
            throw std::runtime_error("Mismatch between struct " + e.name_ + " name and it's type's name");
        }
        if (!e.type()->is_struct()) {
            throw std::runtime_error("Struct " + e.name_ + " has non-struct type");
        }
        if (e.type()->is_struct()->fields_.size() != e.fields_.size()) {
            throw std::runtime_error("Mismatch between struct " + e.name_ + "'s type and it's fields");
        }

        if (!e.scope_) {
            throw std::runtime_error("Struct " + e.name_ + " has no associated scope");
        }
        e.scope_->accept(*this);

        for (auto a:e.fields_) {
            a->accept(*this);
        }
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            auto t0 = e.fields_[i]->type();
            auto t1 = e.type()->is_struct()->fields_[i].type;

            auto n0 = e.fields_[i]->is_vardef()->name_;
            auto n1 = e.type()->is_struct()->fields_[i].name;

            if (t0 != t1 || n0 != n1) {
                throw std::runtime_error("Mismatch between struct " + e.name_ + "'s type and it's arguments");
            }
        }
    }

    virtual void visit(float_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Float number has no type");
        }
        if (!e.type()->is_float()) {
            throw std::runtime_error("Float number has non-float type");
        }
    }

    virtual void visit(vardef_rep& e) override {
        if (e.name_.empty()) {
            throw std::runtime_error("Variable defintion has no name");
        }
        if (!e.type()) {
            throw std::runtime_error("Variable defintion has no type");
        }
        if (e.type()->is_func()) {
            throw std::runtime_error("Variable definiton can't have function type");
        }
    }

    virtual void visit(varref_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Variable references has no type");
        }
        if (!e.def_->is_vardef()) {
            throw std::runtime_error("Variable references a non-vardef expression");
        }

        e.def_->accept(*this);
        if (e.type() != e.def_->type()) {
            throw std::runtime_error("Variable references different type from the variable definiton");
        }
    }

    virtual void visit(let_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Let expression has no type");
        }
        if (!e.var_->is_vardef()) {
            throw std::runtime_error("Let expression's variable references non-vardef expression");
        }
        if (!e.scope_) {
            throw std::runtime_error("Let expression has no associated scope");
        }
        e.var_->accept(*this);
        e.val_->accept(*this);
        e.scope_->accept(*this);
        if (e.scope_->type() != e.type()) {
            throw std::runtime_error("Let expression's type is not the same as its scope's type");
        }
    }

    virtual void visit(binary_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Binary expression has no type");
        }
        e.lhs_->accept(*this);
        e.rhs_->accept(*this);

        if (e.lhs_->type()!= e.rhs_->type() || !e.lhs_->type()->is_float()) {
            throw std::runtime_error("Binary expression has incompatible lhs and rhs types");
        }
        if (e.lhs_->type()!= e.type()) {
            throw std::runtime_error("Binary expression's type is incompatible with the lhs/rhs type");
        }

        // Check if canonical
        if (!(e.lhs_->is_varref() || e.lhs_->is_float()) ||
            !(e.rhs_->is_varref() || e.rhs_->is_float())) {
            throw std::runtime_error("Binary expression's is not canonical");
        }
    }

    virtual void visit(access_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Access expression has no type");
        }
        if (!e.var_->is_varref()) {
            throw std::runtime_error("Cannot access argument of a non-varref expression");
        }
        e.var_->accept(*this);

        if (!e.var_->type()->is_struct()) {
            throw std::runtime_error("Access expression cannot access non-struct type");
        }
        if (e.var_->type()->is_struct()->fields_[e.index_].type != e.type()) {
            throw std::runtime_error("Access expression's type is not the same as the accessed argument's type");
        }
    }

    virtual void visit(create_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Create expression has no type");
        }
        if (!e.type()->is_struct()) {
            throw std::runtime_error("Create expression has non-struct type");
        }
        for (auto a:e.fields_) {
            a->accept(*this);
        }
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            auto t0 = e.fields_[i]->type();
            auto t1 = e.type()->is_struct()->fields_[i].type;
            if (t0 != t1) {
                throw std::runtime_error("Create expression has fields with incorrect types");
            }
        }

        // Check if canonical
        for (auto a:e.fields_) {
            if (!(a->is_varref() || a->is_float())) {
                throw std::runtime_error("create expression's is not canonical");
            }
        }
    }

    virtual void visit(apply_rep& e) override {
        if (!e.type()) {
            throw std::runtime_error("Apply expression has no type");
        }
        if (!e.type()->is_func()) {
            throw std::runtime_error("Apply expression has non-func type");
        }
        for (auto a:e.args_) {
            a->accept(*this);
        }
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            auto t0 = e.args_[i]->type();
            auto t1 = e.type()->is_func()->args_[i].type;
            if (t0 != t1) {
                throw std::runtime_error("Apply expression has args with incorrect types");
            }
        }

        // Check if canonical
        for (auto a:e.args_) {
            if (!(a->is_varref() || a->is_float())) {
                throw std::runtime_error("create expression's is not canonical");
            }
        }
    }
    virtual  void visit(ir_expression& e) override {}
};

struct constant_prop : visitor {
    bool prop_ = false;
    void reset() {
        prop_ = false;
    }
    bool propagation_perfromed() {
        return prop_;
    }
    std::unordered_map<std::string, float> constants;

    void visit(func_rep& e) override {
        e.body_->accept(*this);
        if(e.scope_) {
            e.scope_->accept(*this);
        }
    }

    void visit(struct_rep& e) override {
        if(e.scope_) {
            e.scope_->accept(*this);
        }
    }

    void visit(let_rep& e) override {
        if (e.val_->is_float()) {
            constants.insert({e.var_->is_vardef()->name_, e.val_->is_float()->val_});
        }
        if (auto ref = e.val_->is_varref()) {
            auto it = constants.find(ref->def_->is_vardef()->name_);
            if (it != constants.end()) {
                e.replace_val(std::make_shared<float_rep>(it->second));
            }
            constants.insert({e.var_->is_vardef()->name_, e.val_->is_float()->val_});
        }
        e.val_->accept(*this);

        if (auto bin = e.val_->is_binary()) {
            if (bin->lhs_->is_float() && bin->rhs_->is_float()) {
                auto lhs_val = bin->lhs_->is_float()->val_;
                auto rhs_val = bin->rhs_->is_float()->val_;
                float result;
                switch (bin->op_) {
                    case operation::add : {
                        result = lhs_val + rhs_val;
                        break;
                    }
                    case operation::sub : {
                        result = lhs_val - rhs_val;
                        break;
                    }
                    case operation::mul : {
                        result = lhs_val * rhs_val;
                        break;
                    }
                    case operation::div : {
                        result = lhs_val / rhs_val;
                        break;
                    }
                    default: break;
                }
                e.replace_val(std::make_shared<float_rep>(result));
            }
        }

        e.scope_->accept(*this);
    }

    void visit(binary_rep& e) override {
        if (auto var = e.lhs_->is_varref()) {
            auto it = constants.find(var->def_->is_vardef()->name_);
            if (it != constants.end()) {
                e.replace_lhs(std::make_shared<float_rep>(it->second));
            }
        }
        if (auto var = e.rhs_->is_varref()) {
            auto it = constants.find(var->def_->is_vardef()->name_);
            if (it != constants.end()) {
                e.replace_rhs(std::make_shared<float_rep>(it->second));
            }
        }
    }

    void visit(create_rep& e) override {
        for (unsigned i = 0; i < e.fields_.size(); ++i) {
            if (auto var = e.fields_[i]->is_varref()) {
                auto it = constants.find(var->def_->is_vardef()->name_);
                if (it != constants.end()) {
                    e.replace_field(i, std::make_shared<float_rep>(it->second));
                }
            }
        }
    }

    void visit(apply_rep& e) override {
        for (unsigned i = 0; i < e.args_.size(); ++i) {
            if (auto var = e.args_[i]->is_varref()) {
                auto it = constants.find(var->def_->is_vardef()->name_);
                if (it != constants.end()) {
                    e.replace_arg(i, std::make_shared<float_rep>(it->second));
                }
            }
        }
    }

    void visit(ir_expression& e) override {}
};

struct unused_variables : visitor {
    std::unordered_map<std::string, bool> variables_;

    std::set<std::string> unused_set() {
        std::set<std::string> ret;
        for (auto a: variables_) {
            if(!a.second) {
                ret.insert(a.first);
            }
        }
        return ret;
    }

    void visit(func_rep& e) override {
        for (auto a: e.args_) {
            // assume all function variables are used?
            variables_.insert({a->is_vardef()->name_, true});
        }
        e.body_->accept(*this);
        if(e.scope_) {
            e.scope_->accept(*this);
        }
    }

    void visit(struct_rep& e) override {
        if(e.scope_) {
            e.scope_->accept(*this);
        }
    }

    void visit(varref_rep& e) override {
        variables_.at(e.def_->is_vardef()->name_) = true;
    }

    void visit(vardef_rep& e) override {
        variables_.insert({e.name_, false});
    }

    void visit(let_rep& e) override {
        e.var_->accept(*this);
        e.val_->accept(*this);
        e.scope_->accept(*this);
    }

    void visit(binary_rep& e) override {
        e.lhs_->accept(*this);
        e.rhs_->accept(*this);
    }

    void visit(access_rep& e) override {
        e.var_->accept(*this);
    }

    void visit(create_rep& e) override {
        for (auto a: e.fields_) {
            a->accept(*this);
        }
    }

    void visit(apply_rep& e) override {
        for (auto a: e.args_) {
            a->accept(*this);
        }
    }

    void visit(ir_expression& e) override {}
};

struct eliminate_dead_code : visitor {
    std::set<std::string> unused_vars_;

    eliminate_dead_code(std::set<std::string> unused_vars) : unused_vars_(unused_vars) {}

    void visit(func_rep& e) override {
        while (remove(e.body_->is_let())) {
            e.body_ = e.body_->is_let()->scope_;
        }
        e.body_->accept(*this);

        if(e.scope_) {
            while (remove(e.scope_->is_let())) {
                e.scope_ = e.scope_->is_let()->scope_;
            }
            e.scope_->accept(*this);
        }
    }

    void visit(struct_rep& e) override {
        if (e.scope_) {
            while (remove(e.scope_->is_let())) {
                e.scope_ = e.scope_->is_let()->scope_;
            }
            e.scope_->accept(*this);
        }
    }

    void visit(let_rep& e) override {
        while (remove(e.scope_->is_let())) {
            e.scope_ = e.scope_->is_let()->scope_;
        }
        e.scope_->accept(*this);
    }

    void visit(ir_expression& e) override {}

private:
    bool remove(const let_rep* let) {
        if(!let) {
            return false;
        }
        return unused_vars_.count(let->var_->is_vardef()->name_);
    }
};

struct eliminate_common_subexpressions : visitor {
    std::unordered_map<std::string, ir_ptr> rename_map_; // string -> vardef
    std::vector<std::pair<ir_ptr, ir_ptr>> expressions_; // ir_ptr -> vardef

    void visit(func_rep& e) override {
        e.body_->accept(*this);
        if(e.scope_) {
            e.scope_->accept(*this);
        }
    }

    void visit(struct_rep& e) override {
        e.scope_->accept(*this);
    }

    void visit(varref_rep& e) override {
        if (rename_map_.count(e.def_->is_vardef()->name_)) {
            auto new_def = rename_map_.at(e.def_->is_vardef()->name_);
            e.def_ = new_def;
        }
    }

    void visit(let_rep& e) override {
        e.val_->accept(*this);
        for (auto& a: expressions_) {
            auto exp = a.first;
            auto matching_def = a.second;
            if (compare(exp, e.val_)) {
                rename_map_[e.var_->is_vardef()->name_] = matching_def;
                e.val_ = std::make_shared<varref_rep>(matching_def, matching_def->type());
                e.scope_->accept(*this);
                return;
            }
        }
        expressions_.push_back({e.val_, e.var_});
        e.scope_->accept(*this);
    }

    void visit(binary_rep& e) override {
        e.lhs_->accept(*this);
        e.rhs_->accept(*this);
    }

    void visit(access_rep& e) override {
        e.var_->accept(*this);
    }

    void visit(create_rep& e) override {
        for (auto& a:e.fields_) {
            a->accept(*this);
        }
    }

    void visit(apply_rep& e) override {
        for (auto& a:e.args_) {
            a->accept(*this);
        }
    }

    void visit(ir_expression& e) override {}

private:
    bool compare(ir_ptr e0, ir_ptr e1) {
        if (e0->is_float() && e1->is_float()) {
            if (e0->is_float()->val_ == e1->is_float()->val_) {
                return true;
            }
            return false;
        }
        if (e0->is_varref() && e1->is_varref()) {
            if (e0->is_varref()->def_->is_vardef()->name_ ==
                e1->is_varref()->def_->is_vardef()->name_) {
                return true;
            }
            return false;
        }
        if (e0->is_binary() && e1->is_binary()) {
            if (e0->is_binary()->op_ != e1->is_binary()->op_) {
                return false;
            }

            auto lhs0 = e0->is_binary()->lhs_;
            auto rhs0 = e0->is_binary()->rhs_;
            auto lhs1 = e1->is_binary()->lhs_;
            auto rhs1 = e1->is_binary()->rhs_;

            bool lhs_match = false;
            if ((lhs0->is_varref() && lhs1->is_varref()) &&
                (lhs0->is_varref()->def_->is_vardef()->name_ == lhs1->is_varref()->def_->is_vardef()->name_)) {
                lhs_match = true;
            } else
            if ((lhs0->is_float() && lhs1->is_float()) &&
                  (lhs0->is_float()->val_ == lhs1->is_float()->val_)) {
                lhs_match = true;
            }

            bool rhs_match = false;
            if ((rhs0->is_varref() && rhs1->is_varref()) &&
                (rhs0->is_varref()->def_->is_vardef()->name_ == rhs1->is_varref()->def_->is_vardef()->name_)) {
                rhs_match = true;
            } else
            if ((rhs0->is_float() && rhs1->is_float()) &&
                (rhs0->is_float()->val_ == rhs1->is_float()->val_)) {
                rhs_match = true;
            }
            return lhs_match && rhs_match;
        }
        if (e0->is_access() && e1->is_access()) {
            if ((e0->is_access()->var_->is_varref()->def_->is_vardef()->name_ ==
                 e1->is_access()->var_->is_varref()->def_->is_vardef()->name_) &&
                (e0->is_access()->index_ == e1->is_access()->index_)) {
                return true;
            }
            return false;
        }
        if (e0->is_create() && e1->is_create()) {
            if (e0->is_create()->type() == e1->is_create()->type()) {
                auto& f0 = e0->is_create()->fields_;
                auto& f1 = e1->is_create()->fields_;

                for (unsigned i = 0; i < f0.size(); i++) {
                    bool match = false;
                    if ((f0[i]->is_float() && f1[i]->is_float()) &&
                        (f0[i]->is_float()->val_ == f1[i]->is_float()->val_)) {
                        match = true;
                    }
                    if ((f0[i]->is_varref() && f1[i]->is_varref()) &&
                        (f0[i]->is_varref()->def_->is_vardef()->name_ == f1[i]->is_varref()->def_->is_vardef()->name_)) {
                        match = true;
                    }
                    if(!match) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
        if (e0->is_apply() && e1->is_apply()) {
            if (e0->is_apply()->type() == e1->is_apply()->type()) {
                auto& f0 = e0->is_apply()->args_;
                auto& f1 = e1->is_apply()->args_;

                for (unsigned i = 0; i < f0.size(); i++) {
                    bool match = false;
                    if ((f0[i]->is_float() && f1[i]->is_float()) &&
                        (f0[i]->is_float()->val_ == f1[i]->is_float()->val_)) {
                        match = true;
                    }
                    if ((f0[i]->is_varref() && f1[i]->is_varref()) &&
                        (f0[i]->is_varref()->def_->is_vardef()->name_ == f1[i]->is_varref()->def_->is_vardef()->name_)) {
                        match = true;
                    }
                    if(!match) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        return false;
    };
};
}; //namespace ir