#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "core_arblang.hpp"
using core::typed_var;

struct float_type;
struct struct_type;
struct func_type;

struct typeobj {
    virtual std::string id() const = 0;
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

    struct_type(std::string name, std::vector<typed_var> fields, const std::unordered_map<std::string, type_ptr>& def_types)
    : name_(name) {
        for (auto& f: fields) {
            auto it = def_types.find(f.type);
            if (it == def_types.end()) {
                throw std::runtime_error("Struct \"" + name_ + "\"'s field \"" + f.var + "\" has undefined \"" + f.type + "\" type");
            }
            fields_.emplace_back(f.var, it->second);
        }
    }

    struct_type* is_struct() override {return this;}
};

struct func_type : typeobj {
    std::string name_;
    type_ptr ret_;
    std::vector<field> args_;

    func_type(std::string name, std::string ret, std::vector<typed_var> args, const std::unordered_map<std::string, type_ptr>& def_types)
    : name_(name) {
        for (auto& a: args) {
            auto it = def_types.find(a.type);
            if (it == def_types.end()) {
                throw std::runtime_error("Function \"" + name_ + "\"'s argument \"" + a.var + "\" has undefined \"" + a.type + "\" type");
            }
            args_.emplace_back(a.var, it->second);
        }
        auto it = def_types.find(ret);
        if (it == def_types.end()) {
            throw std::runtime_error("Function \"" + name_ + "\"'s return type \"" + a.type + "\" is undefined");
        }
        ret_ = it->second;
    }

    func_type* is_func() override {return this;}
};
