#include <memory>
#include <string>
#include <vector>

struct real_type;
struct struct_type;
struct func_type;

struct typeobj {
    virtual std::string id() const = 0;
    virtual real_type*   is_real()   {return nullptr;}
    virtual struct_type* is_struct() {return nullptr;}
    virtual func_type*   is_func()   {return nullptr;}
};

using type = std::shared_ptr<typeobj>;

struct real_type : typeobj {
    virtual std::string id() const override {
        return "real";
    }
    real_type* is_real() override {return this;}
};

struct struct_type : typeobj {
    std::string name_;
    std::vector <type> fields_;

    struct_type(std::string name, std::vector <type> fields) : name_(name), fields_(fields) {}

    virtual std::string id() const override {
        return name_;
    }

    struct_type* is_struct() override {return this;}
};

struct func_type : typeobj {
    std::string name_;
    std::vector <type> args_;
    type ret_;

    func_type(std::string name, std::vector <type> args, type ret) : name_(name), args_(args), ret_(ret) {}

    virtual std::string id() const override {
        return "";
    }

    func_type* is_func() override {return this;}

};
