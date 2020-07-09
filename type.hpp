#include <memory>
#include <string>
#include <vector>

struct typeobj {};

using type = std::shared_ptr<typeobj>;

struct real_type: typeobj {};

struct struct_type: typeobj {
  std::vector<type> fields_;
  struct_type(std::vector<type> fields): fields_(fields) {}
};

struct func_type: typeobj {
  std::vector<type> args_;
  type ret_; 
  func_type(std::vector<type> args, type ret): args_(args), ret_(ret) {}
};
