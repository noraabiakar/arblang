#include <memory>
#include <string>
#include <vector>

struct typeobj {
    virtual std::string id() const;
};

using type = std::shared_ptr<typeobj>;

struct real_type : typeobj {
    virtual std::string id() const override {
        return "real";
    }
};

struct struct_type : typeobj {
    std::string name_;
    std::vector <type> fields_;

    struct_type(std::string name, std::vector <type> fields) : name_(name), fields_(fields) {}

    virtual std::string id() const override {
        return name_;
    }
};

struct func_type : typeobj {
    std::vector <type> args_;
    type ret_;

    func_type(std::vector <type> args, type ret) : args_(args), ret_(ret) {}

    virtual std::string id() const override {
        return "";
    }

};
