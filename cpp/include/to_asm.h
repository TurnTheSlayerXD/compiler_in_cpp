
#ifndef TO_ASM
#define TO_ASM

#define SV_ARG(sv) (sv).size(), (sv).data()

#include <utility>

class AsmConverter {
public:
    std::vector<std::string> _stringHeap;

    const Context& ctx;
    std::string out;

    std::vector<std::pair<const Scope*, std::vector<int>>> allocsPerScope;
    std::vector<std::pair<const Scope*, std::vector<int>>> memlocsPerScope;

    const Scope* curScope;

    AsmConverter(const Context& ctx) : ctx{ctx}, curScope{nullptr} {
        form_stack_offsets();
    }

    void add(const auto &l) {
        out += l + "\n";
    }

    std::string_view convert_arg(const InstrArg& arg, int iSize) {
        if (arg.tp == InstrArgType::REG) {
            return cast_reg(arg.as_reg(), iSize);
        }   
        else if (arg.tp == InstrArgType::STACK_LOC) {
            auto scope = arg.as_stack_loc().scope;
            const auto& allocs = get_allocs_at_scope(scope);
            
            auto it = std::find_if(memlocsPerScope.begin(), memlocsPerScope.end(), [&scope](const auto &t) { return t.first == scope; });
            ex_assert(it != memlocsPerScope.end());

            const auto& memlocs = it->second;

            int resultInt = 0;

            int distToScope = 0;
            // Calculating distance to scope
            while (scope != curScope) {
                distToScope += scope->stackOff;
                scope = scope->parentScope;
                ex_assert(scope, "Expected to reach scope");
            }
            resultInt += distToScope;
            // Calculating relative distance to closest stackalloc
            bool found = false;
            for (size_t i = 0; i < allocs.size(); ++i) {
                if (arg.as_stack_loc().stackOff < allocs[i]) {
                    ex_assert(i>0);
                    found = true;
                    ex_assert(i-1< memlocs.size());
                    int matched = memlocs[i-1];
                    matched += arg.as_stack_loc().stackOff - allocs[i-1];
                    resultInt += matched;
                    break;
                }
            }
            ex_assert(found);

            _stringHeap.push_back(str_fmt("%d(%%rsp)", resultInt));
            return std::string_view(_stringHeap.back());
        }
        else if (arg.tp == InstrArgType::LIT) {
            _stringHeap.push_back(str_fmt("$%d", arg.as_lit().val));
            return std::string_view(_stringHeap.back());
        }

        ex_assert(false, "UNREACHABLE");
        return "";
    }

    std::vector<int>& get_allocs_at_scope(const Scope *scope) {
        ex_assert(scope);
        auto it = std::find_if(allocsPerScope.begin(), allocsPerScope.end(), 
                        [&scope] (const auto &p) { return p.first == scope; });
        if (it == allocsPerScope.end()) {
            allocsPerScope.push_back(std::make_pair(scope, std::vector<int>()));
            allocsPerScope.back().second.push_back(0);
            return allocsPerScope.back().second;
        }
        return it->second;
    }

    void form_stack_offsets() {
        for (const auto &i: ctx._instrs) {
            if (i.tp == InstrType::STACKALLOC) {
                std::vector<int>& allocs = get_allocs_at_scope(i.arg1.as_stack_loc().scope);
                allocs.push_back(allocs.back() + i.arg1.as_stack_loc().stackOff);
            }
        }
        for (const auto &[scope, allocs]: allocsPerScope) {
            std::vector<int> reversed;
            for (size_t i = 1; i < allocs.size(); ++i) {
                reversed.push_back(scope->stackOff - allocs[i]);
            }
            memlocsPerScope.push_back(std::make_pair(scope, std::move(reversed)));
        }


        std::cout << "";
    }

    std::string to_asm() {
        for (size_t index = 0; index < ctx._instrs.size(); ++index) {
            const auto &i = ctx._instrs[index];
        switch (i.tp) {

        case InstrType::SCOPE_START: {
            add(str_fmt("#START %.*s", SV_ARG(i.arg1.as_scope()->id)));
            add(str_fmt("subq %rsp, $%d", i.arg1.as_scope()->stackOff));
            curScope = i.arg1.as_scope();
            break;
        }
        case InstrType::SCOPE_END: {
            curScope = curScope->parentScope;
            assert(curScope);
            break;
        }
 
        case InstrType::FUN: {
            add(str_fmt("%.*s:", SV_ARG(i.arg1.as_mark().m)));
            break;
        }
        case InstrType::STACKALLOC: {
            add(str_fmt("#STACKALLOC %d, %.*s", 
                                i.arg1.as_stack_loc().stackOff, 
                                SV_ARG(i.arg1.as_stack_loc().scope->id)));
            break;
        }
        case InstrType::MOV: {
            add(str_fmt("%.*s %.*s, %.*s", 
                        SV_ARG(cast_mov(i.size)), 
                        SV_ARG(convert_arg(i.arg1, i.size)), 
                        SV_ARG(convert_arg(i.arg2, i.size))));
            break;
        }
        case InstrType::GET_PARAM: { 
            std::string_view paramSource;
            switch(i.arg1.as_param_index().p) {
                case 0: paramSource = "%rcx"; break;
                case 1: paramSource = "%rdx"; break;
                case 2: paramSource = "%r8"; break;
                case 3: paramSource = "%r9"; break;
                default: {
                    ex_assert(false && "TODO");
                    break;
                }
            }
            add(str_fmt("%s %.*s, %.*s", 
                        "movq", 
                        SV_ARG(paramSource), 
                        SV_ARG(convert_arg(i.arg2, 8))));
            break;
        }
        case InstrType::PUT_PARAM: { ex_assert(false && "TODO"); break;}
        case InstrType::ADD: { ex_assert(false && "TODO"); break;}
        case InstrType::MARK: { ex_assert(false && "TODO"); break;}
        default: {
            ex_assert(false && "UNREACHABLE"); 
            break;
        }

        }
        }


        return out;
    }


    std::string_view cast_mov(int iSize) {
        switch (iSize) { case 4: return "movl"; case 8: return "movq"; default: assert("UNREACHABLE"); return "";}
    }

    std::string_view cast_reg(Reg reg, int iSize) {
        switch (reg) {
            case Reg::REG_0: switch (iSize) { case 4: return "%ecx"; case 8: return "%rcx"; default: assert(false && "UNREACHABLE"); return "";}
            case Reg::REG_1: switch (iSize) { case 4: return "%edx"; case 8: return "%rdx"; default: assert(false && "UNREACHABLE"); return "";}
            default: assert(false && "UNREACHABLE"); return "";
        }
        
        assert(false && "UNREACHABLE");
        return "";
    }

};

#endif