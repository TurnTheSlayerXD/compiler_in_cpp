#include <iostream>
#include <vector>
#include <algorithm>
#include <format>
#include <ranges>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cassert>

#include "structs.h"

namespace rng = std::ranges;

void __fill_dn_at(SymType cur_non_term, std::vector<First> &dn, const std::vector<Rule> &rules, std::vector<SymType> &visited) {
    assert(!rng::contains(visited, cur_non_term));
    visited.push_back(cur_non_term);
    First t = { .non_term = cur_non_term, .terms = {}};
    for (const auto &r: rules) {
        if (r.head == cur_non_term) {
            assert(r.body.size() > 0);
            if (is_terminal(r.body[0])) {
                t.terms.push_back(r.body[0]);
            }
            else if (r.body[0] != cur_non_term) {
                auto m = rng::find_if(dn, [&r](const auto &t) { return t.non_term == r.body[0]; });
                if (m == dn.end()) {
                    __fill_dn_at(r.body[0], dn, rules, visited);
                    m = dn.end()-1;
                }
                t.terms.insert(t.terms.end(), m->terms.begin(), m->terms.end());
            }
        }
    }
    dn.push_back(std::move(t));
}

std::vector<First> create_first_dn(const std::vector<Rule> &rules) {
    std::vector<First> dn;
    std::vector<SymType> visited;
    for (const auto &r: rules) {
        if (!rng::contains(visited, r.head)) {
            __fill_dn_at(r.head, dn, rules, visited);
        }
    }
    return dn;
}

const std::vector<SymType>& first_for(const std::vector<First>& dn, SymType s) {
    assert(!is_terminal(s));
    auto it = rng::find_if(dn, [&s](const auto &t){ return t.non_term == s; });
    assert(it != dn.end());
    return it->terms;
}


void recurs_close(const Item &parent, ItemSet &set, const std::vector<Rule> &rules, const std::vector<First> &first_dn, std::vector<Item> &visited) {

    if (rng::contains(visited, parent)) return;
    visited.push_back(parent);

    if (parent.is_finished()) return;

    for (const auto &rule: rules) {
        if (parent.cur_sym() == rule.head) {
            std::vector<Item> new_items;
            if (parent.dot_pos+1 < parent.rule->body.size()) {
                SymType next_sym = parent.rule->body[parent.dot_pos+1];
                if (is_terminal(next_sym)) {
                    new_items.push_back({ .rule = &rule, .dot_pos = 0, .lookahead = next_sym });
                }
                else {
                    const auto &ff = first_for(first_dn, next_sym);
                    for (const auto &term: ff) 
                        new_items.push_back({ .rule = &rule, .dot_pos = 0, .lookahead = term });
                }
            }
            else {
                new_items.push_back({ .rule = &rule, .dot_pos = 0, .lookahead = parent.lookahead });
            }
            for (const auto& new_item: new_items) 
                set.add(new_item);
            for (const auto& new_item: new_items) 
                recurs_close(new_item, set, rules, first_dn, visited);
        }
    }
}

void create_closure(ItemSet &item_set, const std::vector<Rule> &rules, const std::vector<First>& first_dn) {
    using enum SymType;
// Assertion checks
{
    assert(item_set.items.size() > 0);
    const auto &fir = item_set.items.front();
    if (fir.dot_pos == 0) {
        assert(rng::all_of(item_set.items, [](const auto &t) { 
            return t.dot_pos == 0; 
        }));
    }
    else {
        assert(fir.dot_pos>0);
        auto sym = fir.rule->body[fir.dot_pos-1];
        assert(rng::all_of(item_set.items, [&sym](const auto &t) { 
            return t.rule->body[t.dot_pos-1] == sym; 
        }));
    }
}

    const std::vector<Item> passed = item_set.items;
    
    std::vector<Item> visited;
    for (const auto& item: passed) {
        recurs_close(item, item_set, rules, first_dn, visited);
    }
}

struct Node {
    SymType sym;
    std::vector<std::shared_ptr<Node>> children;
    
};

struct StackNode {
    size_t state;
    std::shared_ptr<Node> node;
};


void print_tree(const std::shared_ptr<Node> &v) {
    if (v == nullptr) {
        return;
    }

    std::println("{}", to_string(v->sym));
    for (const auto &c: v->children) {
        print_tree(c);
    }
}


ParserTable build_table(const std::vector<Rule> &rules) {
    using enum SymType;

    // Filling up FIRST()
    std::vector<First> first_dn = create_first_dn(rules);

    std::println("First:\n");
    for (const auto &t: first_dn) {
        std::println("{}", to_string(t));
    }
    std::println();
    
    const std::vector<SymType> SYMS = {
        START ,
        EXPR ,
        OPERAND ,
        NUM ,
        PLUS ,
        L_BR ,
        R_BR ,
        _EOF,
    };
    // std::vector<std::array<size_t, sym_count>> table; 
    std::vector<std::unique_ptr<ItemSet>> item_sets;
    ParserTable tbl;

    std::unordered_map<SymType, Action> DEF_ACTION;
    std::unordered_map<SymType, Action> DEF_GOTO;
    for (SymType sym: SYMS) {
        DEF_ACTION[sym] = {.type = ActionType::ERR, };
        DEF_GOTO[sym] = {.type = ActionType::ERR, };
    }

    // Setting up item_set of LR(0) type
    ItemSet init_set = {.items =  {Item{.rule = &rules[0], .dot_pos = 0, . lookahead = _EOF }}}; 

    std::println("Initial item set:\n{}", to_string(init_set));
    create_closure(init_set, rules, first_dn);
    
    std::println("Closed item set:\n{}", to_string(init_set));

    item_sets.push_back(std::make_unique<ItemSet>(std::move(init_set)));
    tbl.action_tbl.push_back(DEF_ACTION);
    tbl.goto_tbl.push_back(DEF_GOTO);

    std::queue<const ItemSet*> q_glob;
    q_glob.push(item_sets.back().get());
  

    size_t SYM_INDEX = SYMS.size();
    const ItemSet *src_set = nullptr;
    while (!q_glob.empty()) {
        if (SYM_INDEX == SYMS.size()) {
            src_set = q_glob.front();
            q_glob.pop();
            SYM_INDEX = 0;
        }

        static_assert(std::is_same_v<decltype(*src_set), const ItemSet&>);
        // assert(rng::all_of(src_set->items, [](const auto &t) { return t.dot_pos < t.rule->body.size(); }));
        // finding all symbols which share the same after dot symbol 
        
        while (SYM_INDEX < SYMS.size()) {
            bool has = rng::any_of(src_set->items, [&SYMS, &SYM_INDEX] (const auto &item){ return !item.is_finished() && item.cur_sym() == SYMS[SYM_INDEX];  });
            if (has) {
                break;
            }
            ++SYM_INDEX;
        }

        if (SYM_INDEX == SYMS.size()) {
            continue;
        }

        SymType SYM_TARG = SYMS[SYM_INDEX];
        ++SYM_INDEX;

        std::println("--------------------------------");
        
        std::println("Processing {}-th SET with SYM_TARG= [{}]", item_sets.size(), to_string(SYM_TARG));

        // Filling up initial set with items which share same SYM at dot pos
        ItemSet cur_item_set = { .items = {} };
        for (const auto& item: src_set->items) {
            if (!item.is_finished() && item.cur_sym() == SYM_TARG) {
                cur_item_set.add(item);
            }
        }

        // Moving dot pos by 1
        for (auto& item: cur_item_set.items) {
            item.dot_pos += 1;
        }

        std::println("Input set:\n{}", to_string(cur_item_set));

        // Creating closure of item set
        create_closure(cur_item_set, rules, first_dn);

        
        std::println("Out closure:\n{}", to_string(cur_item_set));
        
        assert(rng::find(item_sets, src_set, [](const auto &p) { return p.get(); }) != item_sets.end());
        size_t src_set_index = rng::find(item_sets, src_set, [](const auto &p) { return p.get(); }) - item_sets.begin();
        size_t dst_set_index;

        // Appending newly created closure items to queue 
        auto it_new = rng::find_if(item_sets, [&cur_item_set](const auto &t) { return *t == cur_item_set; });
        if(it_new == item_sets.end()) {
            // Saving closure to item_sets
            item_sets.push_back(std::make_unique<ItemSet>(std::move(cur_item_set)));
            q_glob.push(item_sets.back().get());
            dst_set_index = item_sets.size()-1;
            tbl.action_tbl.push_back(DEF_ACTION);
            tbl.goto_tbl.push_back(DEF_GOTO);
        }
        else {
            dst_set_index = it_new - item_sets.begin(); 
        }
        assert(dst_set_index < tbl.action_tbl.size());


        if (is_terminal(SYM_TARG) /*&& tbl.action_tbl[src_set_index][SYM_TARG].type == ActionType::ERR*/) {
            // assert(tbl.action_tbl[src_set_index][SYM_TARG].type == ActionType::ERR);
            tbl.action_tbl[src_set_index][SYM_TARG] = 
                    Action { 
                        .type = ActionType::SHIFT, 
                        ._ = { .shift_to = dst_set_index }
                    };
        }
        else {
            tbl.goto_tbl[src_set_index][SYM_TARG] = 
                Action { 
                    .type = ActionType::GOTO, 
                    ._ = {.goto_ = dst_set_index},
                }; 
        }

        for (const auto& item: item_sets[dst_set_index]->items) {
            if (item.is_finished() && tbl.action_tbl[dst_set_index][item.lookahead].type != ActionType::SHIFT) {
                assert(is_terminal(item.lookahead));
                tbl.action_tbl[dst_set_index][item.lookahead] =
                    Action { 
                        .type = ActionType::REDUCE, 
                        ._ = { .reduce_to = item.rule }
                    };
            }
        }

        std::println("--------------------------------");
        std::println();
    }
    
    
    std::println("{}", to_string(tbl, SYMS));

    return tbl;
}


std::shared_ptr<Node> parse_statement(const std::vector<SymType> &statement, const ParserTable &tbl) {

    assert(statement.size()>0);
    std::vector<StackNode> stack;
    stack.push_back(StackNode{.state = 0, .node = nullptr});

    size_t pos = 0;
    SymType lookup = statement[pos];

    std::shared_ptr<Node> out_node = nullptr;
    ++pos;
    while (true) {
        // if (pos == statement.size()) {
        //     assert(lookup == SymType::_EOF);
        //     assert(stack.size() == 1);
        //     assert(stack[0].state == 0);
        // }
        // assert(stack.size()>0);
        const size_t state = stack.back().state;
        const auto& action = tbl.action_tbl.at(state).at(lookup);
        switch(action.type) {
        case ActionType::SHIFT: {
            stack.push_back({.state = action.shift_to(), .node = std::make_shared<Node>(Node{.sym = lookup, .children = {}})});
            if (pos >= statement.size()) {
                break;
            }
            lookup = statement.at(pos);
            ++pos; 
            break;
        }
        case ActionType::REDUCE: {
            const Rule* r = action.reduce_to();
            assert(r->body.size()>0);
            auto new_node = std::make_shared<Node>(Node{.sym = r->head, .children = {}});
            for (size_t i = r->body.size()-1;;--i) {
                auto node = stack.back().node;
                assert(node->sym == r->body.at(i));
                new_node->children.push_back(node);
                assert(stack.size()>0);
                stack.pop_back();
                if (i == 0) {
                    break;
                }
            }
            rng::reverse(new_node->children);

            assert(stack.size()>0);
            size_t prior_state = stack.back().state;

            if (prior_state == 0 && new_node->sym == SymType::START) {
                out_node = new_node;
                break;
            }

            size_t new_state = tbl.goto_tbl.at(prior_state).at(r->head).goto_();
            stack.push_back(StackNode{.state = new_state, .node = new_node});
            break;
        }
        case ActionType::ERR: {assert(false && "UNREACHABLE"); break;}
        default: {assert(false && "UNREACHABLE"); break;}
        }

        if (out_node) {
            break;
        }
        
    }

    if (!out_node) {
        assert(false && "TODO IF MATCHING FAILED");
    }

    return out_node;
}

int main() {
    using enum SymType;

    const std::vector<Rule> rules = {
        { .head = START, .body = { EXPR }},
        { .head = EXPR, .body = { OPERAND }},
        { .head = EXPR, .body = { L_BR, EXPR, R_BR }},
        { .head = EXPR, .body = { PLUS, EXPR }},
        { .head = EXPR, .body = { EXPR, PLUS, OPERAND }},
        { .head = OPERAND, .body = { NUM }},
    };

    auto tbl = build_table(rules);

    auto out_node = parse_statement({L_BR, NUM, R_BR, PLUS, NUM, _EOF}, tbl);

    print_tree(out_node);


    return 0;
}

