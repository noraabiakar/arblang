#include "transform.hpp"

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
    auto accumulated_weight = std::make_shared<core::binary_expr>(std::make_shared<core::varref_expr>("a"), std::make_shared<core::varref_expr>("w"), core::operation::add);

    auto weighted_i = std::make_shared<core::binary_expr>(i, accumulated_weight, core::operation::mul);
    auto g = std::make_shared<core::binary_expr>(g0, m, operation::mul);

    auto create_curr = std::make_shared<core::create_expr>("current-contrib", std::vector<core::expr_ptr>{weighted_i, g});

    auto let_weighted   = std::make_shared<core::let_expr>(core::typed_var{"w", "float"}, std::make_shared<core::float_expr>(0.1), create_curr);
    auto let_accumulate = std::make_shared<core::let_expr>(core::typed_var{"a", "float"}, std::make_shared<core::float_expr>(3), let_weighted);

    auto current = std::make_shared<core::func_expr>("current-contrib",
                                                     "current",
                                                     std::vector<core::typed_var>{{"p", "param"}, {"s", "state"}, {"c", "cell"}},
                                                     let_accumulate);

    auto block = std::make_shared<core::block_expr>(std::vector<core::expr_ptr>{current_contrib, ion_state, cell, state, param, current});

    block->accept(core_printer);

    auto ir_printer = ir::print(std::cout);
    auto nested_stmt = create_arblang_ir(block);

    std::cout << "\n------------------------------------------------------\n";
    constant_propagate(nested_stmt);
    nested_stmt->accept(ir_printer);

    std::cout << "\n------------------------------------------------------\n";
    elim_dead_code(nested_stmt);
    nested_stmt->accept(ir_printer);

    std::cout << "\n------------------------------------------------------\n";
    elim_common_subexpressions(nested_stmt);
    nested_stmt->accept(ir_printer);

    std::cout << "\n------------------------------------------------------\n";
    elim_dead_code(nested_stmt);
    nested_stmt->accept(ir_printer);

    return 0;
}
