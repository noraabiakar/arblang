#include "visitor.hpp"

ir::ir_ptr create_arblang_ir(std::shared_ptr<core::block_expr> e) {
    std::vector<ir::ir_ptr> statements;
    auto creator = core::create_ir();

    for (auto& s: e->statements_) {
        if (!s->is_struct() && !s->is_func()) {
            throw std::runtime_error("Can only transform struct/func definitions");
        }
        s->accept(creator);
        statements.push_back(creator.statement_);
        creator.reset();
    }

    auto canon = ir::canonical();

    for (auto& s: statements) {
        if (!s->is_func()) { continue; }

        // Get the list of lets created in the function
        s->is_func()->body_->accept(canon);
        auto new_lets = canon.new_lets;

        // Set the type and scope of the last defined let to be varref of the let and it's type
        auto return_val = std::make_shared<ir::varref_rep>(new_lets.back()->is_let()->var_, new_lets.back()->is_let()->var_->type());
        new_lets.back()->is_let()->set_scope(return_val);
        new_lets.back()->is_let()->set_type(return_val->type());

        // Set the types of the lets
        for (int i = new_lets.size()-2; i >=0; --i) {
            new_lets[i]->is_let()->set_type(new_lets[i+1]->type_);
        }

        // Nest the scopes of the lets
        for (unsigned i = 0; i < new_lets.size()-1; ++i) {
            auto& l = new_lets[i];
            auto& n = new_lets[i+1];
            l->is_let()->set_scope(n);
        }

        // Insert them in the deepest scope of the statement
        if (!s->is_func()->body_->is_let()) {
            s->is_func()->set_body(new_lets.front());
        }

        auto let = s->is_func()->body_->is_let();
        while(true) {
            if(!let->scope_->is_let()) {
                let->set_scope(new_lets.front());
                break;
            }
            let = let->scope_->is_let();
        }
    }

    // Nest the rest of the statements
    for (unsigned i= 0; i < statements.size()-1; ++i) {
        auto& s = statements[i];
        auto n = statements[i+1];
        if (auto f = s->is_func()) {
            f->set_scope(n);
        }
        if (auto f = s->is_struct()) {
            f->set_scope(n);
        }
    }

    auto valid = ir::validate();
    statements.front()->accept(valid);

    // return the top-most statement
    return statements.front();
};

void constant_propagate(ir::ir_ptr nested) {
    bool prop = true;

    while(prop) {
        auto propagate = ir::constant_prop();
        nested->accept(propagate);
        prop = propagate.propagation_perfromed();
    }
}

void elim_dead_code(ir::ir_ptr nested) {
    bool elim = true;

    while (elim) {
        auto unused = ir::unused_variables();
        nested->accept(unused);
        auto unused_set = unused.unused_set();

        auto eliminate = ir::eliminate_dead_code(unused_set);
        nested->accept(eliminate);

        elim = unused_set.empty();
    }
}

void elim_common_subexpressions(ir::ir_ptr nested) {
    auto cse = ir::eliminate_common_subexpressions();
    nested->accept(cse);
}