#include "expr.hpp"

int main() {

auto a = std::make_shared<var_expr>("a");
auto e = std::make_shared<var_expr>("e");
auto i = std::make_shared<real_expr>(1);
auto add = std::make_shared<add_expr>(e, i);
auto let = std::make_shared<let_expr>("e", a, add);

//auto real = std::make_shared<real_type>();
//std::vector<pair> l= {pair{"m0", real}, pair{"m1", real}};

//auto str = std::make_shared<struct_expr>("st", l);

auto printer = print(std::cout, 2);

i->accept(printer);
std::cout << std::endl << std::endl;

a->accept(printer);
std::cout << std::endl << std::endl;

add->accept(printer);
std::cout << std::endl << std::endl;

let->accept(printer);
std::cout << std::endl << std::endl;

return 0; 
}
