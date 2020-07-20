#include "visitor.hpp"

int main() {
    using namespace core;
    auto printer = print_core_arblang(std::cout);

    auto ion_state       = std::make_shared<struct_expr>("ion-state",       std::vector<typed_var>{{"iconc", "float"}, {"econc", "float"}});
    auto current_contrib = std::make_shared<struct_expr>("current_contrib", std::vector<typed_var>{{"i", "float"}, {"g", "float"}});
    auto cell  = std::make_shared<struct_expr>("cell",  std::vector<typed_var>{{"v", "float"}, {"temp", "float"}, {"leak", "ion_state"}});
    auto state = std::make_shared<struct_expr>("state", std::vector<typed_var>{{"m", "float"}});
    auto param = std::make_shared<struct_expr>("param", std::vector<typed_var>{{"g0", "float"}, {"erev", "float"}});

    auto v    = std::make_shared<access_expr>("c", "v");
    auto erev = std::make_shared<access_expr>("p", "erev");
    auto g0   = std::make_shared<access_expr>("p", "g0");
    auto m    = std::make_shared<access_expr>("s", "m");

    auto i = std::make_shared<binary_expr>(std::make_shared<binary_expr>(std::make_shared<binary_expr>(v, erev, operation::sub), g0, mul), m, operation::mul);
    auto weighted_i = std::make_shared<binary_expr>(i, std::make_shared<varref_expr>("w"), operation::mul);
    auto g = std::make_shared<binary_expr>(g0, m, operation::mul);

    auto current = std::make_shared<func_expr>("current-contrib",
                                               "current",
                                               std::vector<typed_var>{{"p", "param"}, {"s", "state"}, {"c", "cell"}},
                                               std::make_shared<create_expr>("current_contrib", std::vector<expr_ptr>{weighted_i, g}));

    auto weighted_current  = std::make_shared<let_expr>(typed_var{"w", "float"}, std::make_shared<float_expr>(0.1), current);

    auto block = std::make_shared<block_expr>(std::vector<expr_ptr>{current_contrib, ion_state, cell, state, param, weighted_current});

    block->accept(printer);
    std::cout << "\n------------------------------------------------------\n";

    return 0;
}
