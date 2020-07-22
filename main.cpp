#include "visitor.hpp"

ir::ir_ptr create_arblang_ir(std::shared_ptr<core::block_expr> e) {
    std::vector<ir::ir_ptr> statements;
    auto creator = core::create_ir();

    for (auto& s: e->statements_) {
        if (!s->is_struct() && !s->is_func()) {
            throw std::runtime_error("Can only transform struct/func definitions");
        }
        s->accept(creator);
        statements.push_back(creator.statement_);
        creator.reset();
    }

    auto canon = ir::canonical();

    for (auto& s: statements) {
        if (!s->is_func()) { continue; }

        // Get the list of lets created in the function
        s->is_func()->body_->accept(canon);
        auto new_lets = canon.new_lets;

        // Nest the scopes of the lets
        for (unsigned i = 0; i < new_lets.size()-1; ++i) {
            auto& l = new_lets[i];
            auto& n = new_lets[i+1];
            l->is_let()->set_scope(n);
        }
        auto return_val = std::make_shared<ir::varref_rep>(new_lets.back()->is_let()->var_, new_lets.back()->is_let()->var_->type());
        new_lets.back()->is_let()->set_scope(return_val);

        // Insert them in the deepest scope of the statement
        if (!s->is_func()->body_->is_let()) {
            s->is_func()->set_body(new_lets.front());
        }

        auto let = s->is_func()->body_->is_let();
        while(true) {
            if(!let->scope_->is_let()) {
                let->set_scope(new_lets.front());
                break;
            }
            let = let->scope_->is_let();
        }
    }

    // Nest the rest of the statements
    for (unsigned i= 0; i < statements.size()-1; ++i) {
        auto& s = statements[i];
        auto n = statements[i+1];
        if (auto f = s->is_func()) {
            f->set_scope(n);
        }
        if (auto f = s->is_struct()) {
            f->set_scope(n);
        }
    }

    // return the top-most statement
    return statements.front();
};

int main() {
    using namespace core;
    auto core_printer = core::print(std::cout);

    auto ion_state       = std::make_shared<core::struct_expr>("ion-state",       std::vector<core::typed_var>{{"iconc", "float"}, {"econc", "float"}});
    auto current_contrib = std::make_shared<core::struct_expr>("current-contrib", std::vector<core::typed_var>{{"i", "float"}, {"g", "float"}});
    auto cell  = std::make_shared<core::struct_expr>("cell",  std::vector<core::typed_var>{{"v", "float"}, {"temp", "float"}, {"leak", "ion-state"}});
    auto state = std::make_shared<core::struct_expr>("state", std::vector<core::typed_var>{{"m", "float"}});
    auto param = std::make_shared<core::struct_expr>("param", std::vector<core::typed_var>{{"g0", "float"}, {"erev", "float"}});

    auto v    = std::make_shared<core::access_expr>("c", "v");
    auto erev = std::make_shared<core::access_expr>("p", "erev");
    auto g0   = std::make_shared<core::access_expr>("p", "g0");
    auto m    = std::make_shared<core::access_expr>("s", "m");

    auto i = std::make_shared<core::binary_expr>(std::make_shared<core::binary_expr>(std::make_shared<core::binary_expr>(v, erev, core::operation::sub), g0, mul), m, core::operation::mul);
    auto weighted_i = std::make_shared<core::binary_expr>(i, std::make_shared<core::varref_expr>("w"), core::operation::mul);
    auto g = std::make_shared<core::binary_expr>(g0, m, operation::mul);

    auto create_curr = std::make_shared<core::create_expr>("current-contrib", std::vector<core::expr_ptr>{weighted_i, g});

    auto let_weighted = std::make_shared<core::let_expr>(core::typed_var{"w", "float"}, std::make_shared<core::float_expr>(0.1), create_curr);

    auto current = std::make_shared<core::func_expr>("current-contrib",
                                                     "current",
                                                     std::vector<core::typed_var>{{"p", "param"}, {"s", "state"}, {"c", "cell"}},
                                                     let_weighted);

    auto block = std::make_shared<core::block_expr>(std::vector<core::expr_ptr>{current_contrib, ion_state, cell, state, param, current});

    block->accept(core_printer);

    auto ir_printer = ir::print(std::cout);
    auto nested_stmt = create_arblang_ir(block);

    std::cout << "\n------------------------------------------------------\n";
    nested_stmt->accept(ir_printer);

    return 0;
}
