#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct float_type;
struct struct_type;
struct func_type;

struct typeobj {
    virtual float_type*  is_float()  {return nullptr;}
    virtual struct_type* is_struct() {return nullptr;}
    virtual func_type*   is_func()   {return nullptr;}
};

using type_ptr = std::shared_ptr<typeobj>;

struct field {
    std::string name;
    type_ptr    type;
};

struct float_type : typeobj {
    float_type* is_float() override {return this;}
};

struct struct_type : typeobj {
    std::string name_;
    std::vector<field> fields_;

    struct_type(std::string name, std::vector<field> fields) : name_(name), fields_(fields) {}

    struct_type* is_struct() override {return this;}
};

struct func_type : typeobj {
    std::string name_;
    type_ptr ret_;
    std::vector<field> args_;

    func_type(std::string name, type_ptr ret, std::vector<field> args) : name_(name), ret_(ret), args_(args) {}

    func_type* is_func() override {return this;}
};
