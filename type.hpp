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

    std::string name_;
    virtual std::string name() const {
        return name_;
    }
};

using type_ptr = std::shared_ptr<typeobj>;

struct field {
    std::string name;
    type_ptr    type;
};

struct float_type : typeobj {
    float_type() {
        name_ = "float";
    }
    float_type* is_float() override {return this;}
};

struct struct_type : typeobj {
    std::vector<field> fields_;

    struct_type(std::string name, std::vector<field> fields) : fields_(fields) {name_ = name;}

    struct_type* is_struct() override {return this;}
};

struct func_type : typeobj {
    type_ptr ret_;
    std::vector<field> args_;

    func_type(std::string name, type_ptr ret, std::vector<field> args) : ret_(ret), args_(args) {name_ = name;}

    func_type* is_func() override {return this;}
};
