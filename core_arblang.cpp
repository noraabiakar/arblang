#include "core_arblang.hpp"
#include "visitor.hpp"

namespace core {
void expression::accept(visitor& v) {
    v.visit(*this);
}

void func_expr::accept(visitor& v) {
    v.visit(*this);
}

void struct_expr::accept(visitor& v) {
    v.visit(*this);
}

void float_expr::accept(visitor& v) {
    v.visit(*this);
}

void vardef_expr::accept(visitor& v) {
    v.visit(*this);
}

void varref_expr::accept(visitor& v) {
    v.visit(*this);
}

void let_expr::accept(visitor& v) {
    v.visit(*this);
}

void binary_expr::accept(visitor& v) {
    v.visit(*this);
}

void access_expr::accept(visitor& v) {
    v.visit(*this);
}

void create_expr::accept(visitor& v) {
    v.visit(*this);
}

void apply_expr::accept(visitor& v) {
    v.visit(*this);
}
void block_expr::accept(visitor& v) {
    v.visit(*this);
}

void halt_expr::accept(visitor& v) {
    v.visit(*this);
}
};
