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

std::vector<First> create_first_dn(const std::vector<SymType> &non_terms, const std::vector<const Rule*> &rules) {
    assert(rng::all_of(non_terms, [](const auto& t) { return !is_term(t); }));

    std::queue<SymType> q(non_terms.begin(), non_terms.end());

    std::unordered_map<SymType, std::unordered_set<SymType>> dn;
    while (!q.empty()) {
        SymType sym = q.front();
        q.pop();

        bool had_new_terms = false;
        for (const auto &r: rules) {
            if (r->head == sym) {
                SymType f_rhs = r->body[0];
                if (f_rhs == sym) {
                    continue;
                }
                if (is_term(f_rhs)) {
                    had_new_terms = had_new_terms || !dn[sym].contains(f_rhs);
                    dn[sym].insert(f_rhs);
                }
                else {
                    auto it_rhs = r->body.begin();
                    while (it_rhs != r->body.end()) {
                        bool has_eps = false;
                        for (const auto &t: dn[*it_rhs]) {
                            if (t != SymType::EPS) {
                                had_new_terms = had_new_terms || !dn[sym].contains(t);
                                dn[sym].insert(t);
                            } 
                            else {
                                has_eps = true;
                            }
                        }
                        if (!has_eps) {
                            break;
                        }
                        ++it_rhs;
                    }
                }
            }
        }

        if (had_new_terms || dn[sym].empty()) {
            q.push(sym);
        }
    }

    std::vector<First> first_set;
    for (const auto &[non_t, terms]: dn) {
        first_set.push_back(First{ .non_term = non_t, .terms = std::vector<SymType>(terms.begin(), terms.end()) });
    }

    return first_set;
}

const std::vector<SymType>& first_for(const std::vector<First>& dn, SymType s) {
    assert(!is_term(s));
    auto it = rng::find_if(dn, [&s](const auto &t){ return t.non_term == s; });
    assert(it != dn.end());
    return it->terms;
}


void recurs_close(const Item &parent, ItemSet &set, const std::vector<const Rule*> &rules, const std::vector<First> &first_dn, std::vector<Item> &visited) {

    if (rng::contains(visited, parent)) return;
    visited.push_back(parent);

    if (parent.dot_pos >= parent.rule->body.size()) return;

    for (const auto &rule: rules) {
        if (parent.cur_sym() == rule->head) {
            std::vector<Item> new_items;
            if (parent.dot_pos+1 < parent.rule->body.size()) {
                SymType next_sym = parent.rule->body[parent.dot_pos+1];
                if (is_term(next_sym)) {
                    new_items.push_back(Item {
                        .rule = rule,
                        .dot_pos = 0,
                        .lookahead = next_sym,
                        .trans_set = nullptr,
                        .is_kernel = false,
                    });
                }
                else {
                    const auto &ff = first_for(first_dn, next_sym);
                    for (const auto &term: ff) 
                        new_items.push_back({ 
                            .rule = rule,
                            .dot_pos = 0,
                            .lookahead = term,
                            .trans_set = nullptr,
                            .is_kernel = false,
                        });
                }
            }
            else {
                new_items.push_back({ 
                    .rule = rule,
                    .dot_pos = 0,
                    .lookahead = parent.lookahead,
                    .trans_set = nullptr,
                    .is_kernel = false,
                });
            }
            for (const auto& new_item: new_items) 
                set.add(new_item);
            for (const auto& new_item: new_items) 
                recurs_close(new_item, set, rules, first_dn, visited);
        }
    }
}

void close_set(ItemSet &item_set, const std::vector<const Rule*> &rules, const std::vector<First>& first_set) {
    const std::vector<Item> passed = item_set.items;
    std::vector<Item> visited;
    for (const auto& item: passed) {
        recurs_close(item, item_set, rules, first_set, visited);
    }
}

bool can_be_merged(const ItemSet& a, const ItemSet& b) {
    for (const auto &i: a.items){
        if (i.is_kernel) {
            auto b_it = rng::find_if(b.items, 
                [&i](const auto &j){ return j.is_kernel && 
                                            j.rule == i.rule && 
                                            j.dot_pos == i.dot_pos; });
            if (b_it == b.items.end()) {
                return false;
            }
            
        }    
    }
    for (const auto &i: b.items){
        if (i.is_kernel) {
            auto a_it = rng::find_if(a.items, 
                [&i](const auto &j){ return j.is_kernel && 
                                            j.rule == i.rule && 
                                            j.dot_pos == i.dot_pos; });
            if (a_it == a.items.end()) {
                return false;
            }
        }    
    }
    for (const auto &i: a.items) {
        if (i.is_reduce() && i.is_kernel) {
            bool b_has_dif_reduce = rng::find_if(b.items, 
                [&i](const auto &j) { return j.is_kernel && 
                                            j.is_reduce() && 
                                            j.rule->body == i.rule->body && 
                                            j.lookahead == i.lookahead && 
                                            j.rule->head != i.rule->head; }) != b.items.end();
            if (b_has_dif_reduce) {
                return false;
            }
        }
    }
    for (const auto &i: b.items) {
        if (i.is_reduce() && i.is_kernel) {
            bool a_has_dif_reduce = rng::find_if(a.items, 
                [&i](const auto &j) { return j.is_kernel && 
                                            j.is_reduce() && 
                                            j.rule->body == i.rule->body && 
                                            j.lookahead == i.lookahead && 
                                            j.rule->head != i.rule->head; }) != a.items.end();
            if (a_has_dif_reduce) {
                return false;
            }
        }
    }

    return true;
}

void merge_item_sets(ItemSet& g_set, ItemSet &set) {
    if (g_set.is_complete) {
        for (const auto &i: set.items) {
            if (i.is_kernel || i.is_reduce()) {
                auto g_it = rng::find_if(g_set.items, 
                    [&i](const auto &j) { return j.rule == i.rule && j.dot_pos == i.dot_pos; });
                assert(g_it != g_set.items.end());
                Item cp = i;
                cp.trans_set = g_it->trans_set;
                g_set.add(cp);
            }
        }
    }
    else {
        for (const auto &i: set.items) {
            auto g_it = rng::find_if(g_set.items, 
                    [&i](const auto &j) { return j.rule == i.rule && j.dot_pos == i.dot_pos; });
            assert(g_it != g_set.items.end());
            Item cp = i;
            cp.trans_set = g_it->trans_set;
            g_set.add(cp);
        }
    }
    assert(set.from_set);
    for (auto &i: set.from_set->items) {
        if (i.trans_set == &set) {
            i.trans_set = &g_set;
        }
    }
}

ParserTable build_table(const std::vector<SymType> &non_terms, const std::vector<const Rule*> &rules) {
    using enum SymType;
    assert(rng::all_of(rules, [](const auto &r) {return r->body.size() > 0; } ));
    assert(rng::all_of(rules, [](const auto &r) {return !is_term(r->head); } ));
    assert(rng::all_of(non_terms, [](const auto &t) {return !is_term(t); } ));
    assert(rng::all_of(rules, [&non_terms](const auto &r) {return rng::contains(non_terms, r->head); } ));

    // Filling up FIRST()
    std::vector<First> first_set = create_first_dn(non_terms, rules);
    
    if (0) {
        std::println("First:\n");
        for (const auto &t: first_set) {
            std::println("{}", to_string(t));
        }
        std::println();
    }

    std::vector<std::unique_ptr<ItemSet>> todo_list;
    std::vector<std::unique_ptr<ItemSet>> done_list;
    std::queue<ItemSet*> inc_list;

    auto init_rule = rng::find_if(rules, [](const auto&r) { return r->head == START; });
    assert(init_rule != rules.end());

    auto init_set = std::make_unique<ItemSet>(
        ItemSet{
            .from_set = nullptr, 
            .items = {},
            .is_complete = false,
        }
    );
    init_set->add(Item {
                    .rule = *init_rule,
                    .dot_pos = 0,
                    .lookahead = _EOF,
                    .trans_set = nullptr,
                    .is_kernel = true,
                });

    todo_list.push_back(std::move(init_set));

    size_t n_states = 0;
    
    while (!todo_list.empty() || !inc_list.empty()) {

        while (!todo_list.empty()) {
            auto set = std::move(todo_list.back());
            todo_list.pop_back();
            close_set(*set.get(), rules, first_set);
            bool is_discarded = false;
            if (1) {
                for (const auto& g_set: done_list) {
                    if (can_be_merged(*set, *g_set)) {
                        merge_item_sets(*g_set, *set);
                        is_discarded = true;
                        break;
                    }
                }
            }
            if (!is_discarded) {
                set->state_num = n_states++;
                inc_list.push(set.get());
                done_list.push_back(std::move(set));
            }
        }

        if (!inc_list.empty()) {
            auto set = inc_list.front();
            inc_list.pop();

            for (auto &i: set->items) {
                if (i.trans_set || i.is_reduce()) continue;
                auto new_set = std::make_unique<ItemSet>(ItemSet {
                    .from_set = set,
                    .items  = {},
                    .is_complete = false,
                });
                if (i.cur_sym() == R_BR && n_states == 10) {
                    std::println();
                }
                for (auto &j: set->items) {
                    if (!j.is_reduce() && i.cur_sym() == j.cur_sym()) {
                        j.trans_set = new_set.get();
                        Item k = {
                            .rule = j.rule,
                            .dot_pos = j.dot_pos+1,
                            .lookahead = j.lookahead,
                            .trans_set = nullptr,
                            .is_kernel = true,
                        };
                        new_set->add(k);
                    }
                }
                todo_list.push_back(std::move(new_set));
            }
            for (const auto &i: set->items) {
                if (!i.is_reduce()) {
                    assert(i.trans_set);
                }
            }
            set->is_complete = true;
        }
    }
    // Table construction
    ParserTable tbl = {
        .goto_tbl = std::vector<std::unordered_map<SymType, Action>>(n_states),
        .action_tbl = std::vector<std::unordered_map<SymType, Action>>(n_states),
    };

    std::unordered_map<size_t, std::unordered_map<SymType, Item>> i_cache;
    for (const auto &set: done_list) {
        for (const auto& i: set->items) {
            if (i.is_reduce()) {
                auto &row = tbl.action_tbl[set->state_num];
                auto new_action = Action { 
                    .type = ActionType::REDUCE, 
                    ._ = { .reduce_to = i.rule } 
                };
                auto it = row.find(i.lookahead);
                if (1){
                    if (it != row.end()) {
                        if (it->second.type == ActionType::REDUCE && it->second.reduce_to() != i.rule) {
                            std::println("Detected REDUCE-REDUCE conflict at state [{}] lookahead [{}].\nCONFLICTING ITEMS:\nOld item: {}\nNew item: {}", 
                                set->state_num, 
                                to_string(i.lookahead),
                                to_string(i_cache.at(set->state_num).at(i.lookahead)),
                                to_string(i)
                            );
                        }
                        else {
                            std::println("Detected REDUCE-SHIFT conflict at state [{}] lookahead [{}].\nCONFLICTING ITEMS:\nOld item: {}\nNew item: {}", 
                                set->state_num,
                                to_string(i.lookahead),
                                to_string(i_cache.at(set->state_num).at(i.lookahead)),
                                to_string(i)
                            );
                        }
                    }
                }
                if (it == row.end() || it->second.type != ActionType::SHIFT) {
                    row[i.lookahead] = new_action;
                    i_cache[set->state_num][i.lookahead] = i;
                }
            }
            if (i.is_shift()) {
                if (is_term(i.cur_sym())) {
                    auto &row = tbl.action_tbl[set->state_num];
                    assert(i.trans_set && "SHOULD EXIST");
                    auto it = row.find(i.cur_sym());
                    assert(it == row.end() || (it->second.type == ActionType::SHIFT && it->second.shift_to() == i.trans_set->state_num) || (it->second.type == ActionType::REDUCE));
                    if (1) {
                        if (it != row.end() && it->second.type == ActionType::REDUCE) {
                            std::println("Detected SHIFT-REDUCE conflict at state [{}] lookahead [{}].\nCONFLICTING ITEMS:\nOld: {}\nNew: {}", 
                                set->state_num, 
                                to_string(i.cur_sym()),
                                to_string(i_cache.at(set->state_num).at(i.cur_sym())),
                                to_string(i)
                            );
                        }
                    }
                    row[i.cur_sym()] = Action { 
                        .type = ActionType::SHIFT, 
                        ._ = { .shift_to = i.trans_set->state_num }, 
                    };
                    i_cache[set->state_num][i.cur_sym()] = i;
                }
                else {
                    auto& goto_row = tbl.goto_tbl[set->state_num];
                    auto it = goto_row.find(i.cur_sym());
                    assert(it == goto_row.end() || it->second.goto_() == i.trans_set->state_num);
                    goto_row[i.cur_sym()] = Action {
                        .type = ActionType::GOTO,
                        ._ = { .goto_ = i.trans_set->state_num },
                    };
                    // i_cache[set->state_num][i.cur_sym()] = goto_row[i.cur_sym()];
                }
            }
        }
    }

    if (0) {
        for (const auto &set: done_list) {
            if (set->from_set) {
                const auto &item = set->items[0];
                std::println("Item set {} ( from {}, goto [ {} ] ):", set->state_num, set->from_set->state_num, to_string(item.rule->body[item.dot_pos-1]));
            }
            else {
                std::println("Item set {}:", set->state_num);
            }
            std::println("{}", to_string(*set));
        }
    }

    return tbl;
}

struct Node {
    SymType sym;
    std::vector<std::shared_ptr<Node>> children;
    
};

struct StackItem {
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

std::shared_ptr<Node> parse_statement(const std::vector<SymType> &text, const ParserTable &tbl) {
    assert(rng::all_of(text, [](const auto &t) { return is_term(t); }));

    std::vector<StackItem> stack;
    stack.push_back(StackItem{.state = 0, .node = nullptr});

    auto it = text.begin();
    const SymType *lookup = nullptr;
    while (true) {
        if (lookup == nullptr) {
            if (it == text.end()) {
                if (it == text.begin()) {
                    std::println(stderr, "EMPTY INPUT!");
                }
                else {
                    std::println(stderr, "EXPECTED MORE SYMBOLS AFTER [{}]", to_string(*(it-1)));
                }
                return nullptr;
            }
            lookup = &*it;
            ++it;
        } 

        size_t state = stack.back().state;
        
        const auto &act_row = tbl.action_tbl.at(state);
        auto it_a = act_row.find(*lookup);
        if (it_a == act_row.end()) {
            it_a = act_row.find(SymType::EPS);
            if (it_a == act_row.end()) {
                std::println(stderr, 
                    "Unexpected token [{}] at pos {}\nExpr: {}",
                    to_string(*lookup), 
                    it - text.begin(),
                    to_string(text)
                );
                std::print(stderr, "Expected one of these tokens [");
                for (const auto &[k, _]: tbl.action_tbl.at(state)) {
                    std::print(stderr, "{}, ", to_string(k));
                }
                std::println(stderr, "]");
                return nullptr;
            }
            static const SymType eps = SymType::EPS;
            lookup = &eps;
            --it;
        }

        const auto& action = it_a->second;
        switch(action.type) {
        case ActionType::SHIFT: {
            stack.push_back(StackItem {
                .state = action.shift_to(), 
                .node = std::make_shared<Node>(Node{.sym = *lookup, .children = {}})
            });
            lookup = nullptr;
            break;
        }
        case ActionType::REDUCE: {
            const Rule* r = action.reduce_to();
            auto new_node = std::make_shared<Node>(Node{
                .sym = r->head, 
                .children = std::vector<std::shared_ptr<Node>>(r->body.size(), nullptr)
            });
            for (size_t i = 0; i < r->body.size(); ++i) {
                auto node = stack.back().node;
                assert(r->body.size() >= 1+i);
                assert(node->sym == r->body.at(r->body.size()-1-i));
                new_node->children[r->body.size()-1-i] = node;
                assert(stack.size()>0);
                stack.pop_back();
            }
            assert(stack.size()>0);
            size_t prior_state = stack.back().state;

            if (prior_state == 0 && new_node->sym == SymType::START) {
                if (*lookup == SymType::_EOF) {
                    return new_node;
                }

                std::println(stderr, "Unexpected end token {}", to_string(*lookup));
                return nullptr;
            }

            size_t new_state = tbl.goto_tbl.at(prior_state).at(r->head).goto_();
            stack.push_back(StackItem{.state = new_state, .node = new_node});
            break;
        }
        case ActionType::ERR: { assert(false && "UNREACHABLE"); break; }
        default: { assert(false && "UNREACHABLE"); break; }
        }
    }

    return nullptr;
}

int main() {
    using enum SymType;

    std::initializer_list<Rule> st_rules = {
        { .head = START,   .body = { EXPR }},
        { .head = EXPR,    .body = { EPS }},
        { .head = EXPR,    .body = { OPERAND }},
        { .head = OPERAND, .body = { L_BR, OPERAND, R_BR }},
        { .head = OPERAND, .body = { NUM }},
        { .head = OPERAND, .body = { OPERAND, PLUS, OPERAND }},
    };

    std::vector<const Rule*> rules(st_rules.size());
    for (size_t i = 0; i < st_rules.size(); ++i) {
        rules[i] = &*(st_rules.begin()+i);
    }

    auto tbl = build_table({START, EXPR, OPERAND}, rules);
    auto out_node = parse_statement({L_BR, NUM, PLUS, NUM, R_BR, PLUS, L_BR, NUM, R_BR, _EOF}, tbl);
    if (!out_node) {
        return 69;
    }

    print_tree(out_node);


    return 0;
}