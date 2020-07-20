#include "core_arblang.hpp"

/*void test() {
    auto a = std::make_shared<var_expr>("a");
    auto e = std::make_shared<var_expr>("e");
    auto i = std::make_shared<real_expr>(1);
    auto add = std::make_shared<add_expr>(e, i);
    auto let = std::make_shared<let_expr>("e", a, add);

    auto real = std::make_shared<real_type>();
    std::vector<pair> l = {pair{"m0", real}, pair{"m1", real}};
    auto str = std::make_shared<struct_expr>("st", l);


    auto m0 = std::make_shared<var_expr>("m0");
    auto m1 = std::make_shared<access_expr>(str, "m0");
    auto sub = std::make_shared<sub_expr>(m0, m1);
    auto fun = std::make_shared<func_expr>("fun", real, l, sub);

    auto printer = print(std::cout);

    i->accept(printer);
    std::cout << std::endl << std::endl;

    a->accept(printer);
    std::cout << std::endl << std::endl;

    add->accept(printer);
    std::cout << std::endl << std::endl;

    let->accept(printer);
    std::cout << std::endl << std::endl;

    str->accept(printer);
    std::cout << std::endl << std::endl;

    fun->accept(printer);
    std::cout << std::endl << std::endl;
}*/

/*
int main() {
    auto printer = print(std::cout);
    auto ir_printer = print_ir(std::cout);

    auto real = std::make_shared<real_type>();

    auto ion_state       = std::make_shared<struct_expr>("ion-state",       std::vector<pair>{{"iconc", real}, {"econc", real}});
    auto current_contrib = std::make_shared<struct_expr>("current_contrib", std::vector<pair>{{"i", real}, {"g", real}});
    auto cell  = std::make_shared<struct_expr>("cell",  std::vector<pair>{{"v", real}, {"temp", real}, {"leak", ion_state->type_}});
    auto state = std::make_shared<struct_expr>("state", std::vector<pair>{{"m", real}});
    auto param = std::make_shared<struct_expr>("param", std::vector<pair>{{"g0", real}, {"erev", real}});

    auto v    = std::make_shared<access_expr>(cell, "v");
    auto erev = std::make_shared<access_expr>(param, "erev");
    auto g0   = std::make_shared<access_expr>(param, "erev");
    auto m    = std::make_shared<access_expr>(state, "m");

    auto i = std::make_shared<mul_expr>(std::make_shared<mul_expr>(std::make_shared<sub_expr>(v, erev), g0), m);
    auto g = std::make_shared<mul_expr>(g0, m);

    auto current = std::make_shared<func_expr>("current",
                                               current_contrib->type_,
                                               std::vector<pair>{{"p", param->type_}, {"s", state->type_}, {"c", cell->type_}},
                                               std::make_shared<create_expr>(current_contrib, std::vector<expr>{i, g}));

    auto block = std::make_shared<block_expr>(std::vector<expr>{current_contrib, ion_state, cell, state, param, current});

    block->accept(printer);
    std::cout << "\n------------------------------------------------------\n";

    auto nest = std::make_shared<nested_expr>(block.get());
    nest->accept(ir_printer);
    std::cout << "\n------------------------------------------------------\n";

    return 0;
}
*/

int main() {
    using namespace core;
    auto printer = print(std::cout);

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
    auto g = std::make_shared<binary_expr>(g0, m, operation::mul);

    auto current = std::make_shared<func_expr>("current",
                                               std::vector<typed_var>{{"p", "param"}, {"s", "state"}, {"c", "cell"}},
                                               std::make_shared<create_expr>("current_contrib", std::vector<expr>{i, g}));

    auto block = std::make_shared<block_expr>(std::vector<expr>{current_contrib, ion_state, cell, state, param, current});

    block->accept(printer);
    std::cout << "\n------------------------------------------------------\n";

    return 0;
}
