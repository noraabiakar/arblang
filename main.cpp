#include "expr.hpp"

int main() {

auto a = var_expr("a");

auto i = std::make_shared<real_expr>(1);

auto printer = print(std::cout, 2);
i->accept(printer);

return 0; 
}
