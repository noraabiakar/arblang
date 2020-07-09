#include <memory>
#include <string>
#include <vector>

#include "type.hpp"

struct expression {};

using expr = std::shared_ptr<expression>;

struct func_expr: expression {
  std::string name_;
  std::vector<std::string> args_; 
  expr body_; 
  type type_;

  func_expr(type ret, std::string name, std::vector<std::pair<std::string,type>> args, expr body)
    :name_(name), body_(body) {
    std::vector<type> types; 
    for (auto a: args) {
      args_.push_back(a.first);
      types.push_back(a.second);
    }
    type_ = std::make_shared<func_type>(std::move(types), ret);
  }

  type signature() {
    return type_;
  }
};

struct struct_expr: expression {
  std::string name_; 
  std::vector<std::string> fields_;
  type type_;
  
  struct_expr(std::string name, std::vector<std::pair<std::string, type>> fields): name_(name) {
    std::vector<type> types; 
    for (auto a: fields) {
      fields_.push_back(a.first);
      types.push_back(a.second);
    }
    type_ = std::make_shared<struct_type>(std::move(types));
  } 

  type signature() {
    return type_;
  }
};

struct real_expr: expression {
  double val_; 
  real_expr(double val): val_(val) {}
};

struct var_expr: expression {
  std::string name_;
  var_expr(std::string name): name_(name) {}
};

struct let_expr: expression {
  std::string var_; 
  expr val_;
  expr body_;
  let_expr(std::string var, expr val, expr body): var_(var), val_(val), body_(body) {}
};

struct add_expr: expression {
  expr lhs_; 
  expr rhs_; 
  add_expr(expr lhs, expr rhs): lhs_(lhs), rhs_(rhs) {}
};

struct sub_expr: expression {
  expr lhs_; 
  expr rhs_; 
  sub_expr(expr lhs, expr rhs): lhs_(lhs), rhs_(rhs) {}
};

struct mul_expr: expression {
  expr lhs_; 
  expr rhs_; 
  mul_expr(expr lhs, expr rhs): lhs_(lhs), rhs_(rhs) {}
};

struct div_expr: expression {
  expr lhs_; 
  expr rhs_; 
  div_expr(expr lhs, expr rhs): lhs_(lhs), rhs_(rhs) {}
};

struct access_expr: expression {
  expr struct_; 
  std::string field_;
  access_expr(expr str, std::string field): struct_(str), field_(field) {}
};

struct create_expr: expression {
  expr struct_;
  std::vector<expr> fields_; 
  create_expr(expr str, std::vector<expr> fields): struct_(str), fields_(fields) {}
};

struct apply_expr: expression {
  expr func_; 
  std::vector<expr> args_;
  apply_expr(expr func, std::vector<expr> args): func_(func), args_(args) {}
};
