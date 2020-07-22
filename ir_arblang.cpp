#include "ir_arblang.hpp"
#include "visitor.hpp"

namespace ir {

func_rep::func_rep(std::string name, type_ptr ret, std::vector<ir_ptr> args, ir_ptr body) : name_(name), args_(args), body_(body) {
    std::vector<field> typed_args;
    for (auto& a: args) {
        if (auto vr = a->is_vardef()) {
            typed_args.push_back({vr->name_, vr->type_});
        } else {
            throw std::runtime_error("function argument of non-varref type");
        }
    }
    type_ = std::make_shared<func_type>(name, ret, typed_args);
}

struct_rep::struct_rep(std::string name, std::vector<ir_ptr> fields) : name_(name), fields_(fields) {
    std::vector<field> typed_fields;
    for (auto& f: fields) {
        if (auto vr = f->is_vardef()) {
            typed_fields.push_back({vr->name_, vr->type_});
        } else {
            throw std::runtime_error("struct field of non-varref type");
        }
    }
    type_ = std::make_shared<struct_type>(name, typed_fields);
}

void ir_expression::accept(visitor& v) {
    v.visit(*this);
}

void struct_rep::accept(visitor& v) {
    v.visit(*this);
}

void func_rep::accept(visitor& v) {
    v.visit(*this);
}

void float_rep::accept(visitor& v) {
    v.visit(*this);
}

void vardef_rep::accept(visitor& v) {
    v.visit(*this);
}

void varref_rep::accept(visitor& v) {
    v.visit(*this);
}

void let_rep::accept(visitor& v) {
    v.visit(*this);
}

void binary_rep::accept(visitor& v) {
    v.visit(*this);
}

void access_rep::accept(visitor& v) {
    v.visit(*this);
}

void create_rep::accept(visitor& v) {
    v.visit(*this);
}

void apply_rep::accept(visitor& v) {
    v.visit(*this);
}
};
