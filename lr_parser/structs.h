#ifndef STRUCTS_H
#define STRUCTS_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <format>
#include <ranges>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

namespace rng = std::ranges;

enum class SymType {
#define TERM_SYMS\
    X(PLUS, "+")\
    X(NUM, "n")\
    X(_EOF, "eof")\
    X(L_BR, "(")\
    X(R_BR, ")")\
    X(EPS, "eps")
    #define X(name, _) name,
    TERM_SYMS
    #undef X

#define NONTERM_SYMS\
    X(START, "S")\
    X(EXPR, "E")\
    X(OPERAND, "T")
    #define X(name, _) name,
    NONTERM_SYMS
    #undef X
};

struct Rule {
    SymType head;
    std::vector<SymType> body;
};

struct First {
    SymType non_term;
    std::vector<SymType> terms;
};

struct ItemSet;

struct Item {
    const Rule* rule;
    size_t dot_pos;
    SymType lookahead;
    ItemSet* trans_set;
    bool is_kernel;

    bool operator==(const Item &rhs) const {
        return rule == rhs.rule && dot_pos == rhs.dot_pos && lookahead == rhs.lookahead;
    }

    bool is_shift() const {
        return dot_pos < rule->body.size();
    }
    bool is_reduce() const {
        return dot_pos >= rule->body.size();
    }

    SymType cur_sym() const {
        assert(dot_pos < rule->body.size());
        return rule->body[dot_pos];
    }

};

struct ItemSet {
    size_t state_num;
    ItemSet *from_set;
    std::vector<Item> items;
    bool is_complete;

    void add(const Item &item) {
        if (!rng::contains(items, item)) {
            items.push_back(item);
        }
    }
    
    SymType goto_sym() {
        auto first_kern_it = rng::find_if(items, [](const auto &i) { return i.is_kernel; });
        assert(first_kern_it != items.end());
        SymType sym = (*first_kern_it).rule->body[(*first_kern_it).dot_pos-1];
        for (const auto &i: items) {
            if (i.is_kernel) {
                assert(i.rule->body[i.dot_pos-1] == sym);
            }
        }
        return sym;
    }

    bool operator==(const ItemSet& rhs) const {
        if (items.size() != rhs.items.size()) {
            return false;
        }
        for (const auto& item: items) {
            if (!rng::contains(rhs.items, item)) {
                return false;
            }
        }
        return true;
    }
};

using FollowSet = std::unordered_map<SymType, std::unordered_set<SymType>>;

bool is_term(const SymType& sym) {
    using enum SymType;
    switch(sym) {
    #define X(name, _) case name:
    TERM_SYMS
    #undef X
    return true;

    #define X(name, _) case name:
    NONTERM_SYMS
    #undef X
    return false;

    default: assert(false && "UNREACHABLE"); return false;
    }
}


std::string to_string(const SymType& s) {
    using enum SymType;
    switch(s) {
    #define X(cs, str) case cs: return str;
        TERM_SYMS
        NONTERM_SYMS
    #undef X
    default: assert(false && "UNREACHABLE"); return "";
    }
}

enum class ActionType {
    SHIFT,
    REDUCE,
    GOTO,
    ERR,
};

struct Action {
    ActionType type;
    struct {
        const Rule* reduce_to;
        size_t shift_to;
        size_t goto_;
    } _;
    const Rule* reduce_to() const {
        assert(type == ActionType::REDUCE);
        return _.reduce_to;
    }
    size_t shift_to() const {
        assert(type == ActionType::SHIFT);
        return _.shift_to;
    }

    size_t goto_() const {
        assert(type == ActionType::GOTO);
        return _.goto_;
    }
};

struct ParserTable {
    std::vector<std::unordered_map<SymType, Action>> goto_tbl;
    std::vector<std::unordered_map<SymType, Action>> action_tbl;
};


std::string to_string(const Action& act) {
    switch (act.type) {
        case ActionType::SHIFT: return std::format("S[{}]", act.shift_to());
        case ActionType::REDUCE: return std::format("R[{}]", to_string(act.reduce_to()->head));
        case ActionType::GOTO: return std::format("{}", act.goto_());
        case ActionType::ERR: return "_";
        default: assert(false && "UNREACHABLE"); return "";
    }
}

std::string to_string(const Rule& r) {
    std::string s;
    s += std::format("{} ->", to_string(r.head));
    for (const SymType& sym: r.body) {
        s += std::format(" {}", to_string(sym));
    }
    return s;
}

std::string to_string(const Item& item) {

    std::string rule_with_dot = std::format("{} ->", to_string(item.rule->head));
    for (size_t i = 0; i < item.rule->body.size(); ++i) {
        if (item.dot_pos == i) {
            rule_with_dot += " .";
        }
        rule_with_dot += std::format(" {}", to_string(item.rule->body[i]));
    }

    if (item.dot_pos >= item.rule->body.size()) {
        rule_with_dot += " .";
    }

    return std::format("Item [ {} ]  look_d=[ {} ] {}", rule_with_dot, to_string(item.lookahead), item.is_kernel ? "*" : "");
}

std::string to_string(const ItemSet &set) {
    std::string s;
    size_t i = 1;
    for (const Item& item: set.items) {
        s += std::format("{} {}\n", i++, to_string(item));
    }
    return s;
}

std::string to_string(const First &f) {
    if (!f.terms.empty()) {
        std::string term_str;
        auto it = f.terms.begin();
        term_str += std::format("[{}", to_string(*it));
        while (true) {
            ++it;
            if (it == f.terms.end()) break;
            term_str += std::format(",{}", to_string(*it));
        }
        term_str += "]";
        return std::format("first({}, {})", to_string(f.non_term), term_str);
    }
    
    return std::format("first({}, {})", to_string(f.non_term), "[]");
}

std::string to_string(const auto &vec) {
    auto it = vec.begin();
    if (it == vec.end()) {
        return "";
    }
    std::string s = to_string(*it);
    while (true) {
        ++it;
        if (it == vec.end()) break;
        s += std::format(", {}", to_string(*it));
    }
    return s;
}


std::string to_string(ParserTable& tbl, const std::vector<SymType> &SYMS) {

    std::vector<SymType> sorted_syms(SYMS.begin(), SYMS.end());
    rng::sort(sorted_syms, [](const auto &l, const auto &r) { return is_term(l) && !is_term(r); });

    std::string out;
    out += "PARSER TABLE:\n";
    out += std::format("{:>2}", " ");
    for (auto sym: sorted_syms) {
        out += std::format("{:>8}", to_string(sym));
    }
    out += "\n";

    size_t st_count = tbl.goto_tbl.size();;
    for (size_t i = 0; i < st_count; ++i) {
        out += std::format("{:>3}", i);
        // static_assert(std::is_same_v<decltype(tbl.goto_tbl[i][SYMS[0]]), size_t>); 
        if (is_term(sorted_syms[0])) {
            out += std::format("{:>7}", to_string(tbl.action_tbl[i][sorted_syms[0]]));
        }
        else {
            out += std::format("{:>7}", to_string(tbl.goto_tbl[i][sorted_syms[0]]));
        }
        for (size_t s = 1; s < sorted_syms.size(); ++s) {
            if (is_term(sorted_syms[s])) {
                out += std::format("{:>8}", to_string(tbl.action_tbl[i][sorted_syms[s]]));
            }
            else {
                out += std::format("{:>8}", to_string(tbl.goto_tbl[i][sorted_syms[s]]));
            }
        }
        out += "\n";
    }
    out += "\n";
    return out;
} 


#endif