
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <help.h>

constexpr int PTR_SIZE = 8;
constexpr int FUN_PARAMS_STACK_OFF = 40;

enum class InstrType {
    STACKALLOC,
    MOV,
    GET_PARAM,
    PUT_PARAM,
    ADD,
    FUN,
    MARK,
    SCOPE_START,
    SCOPE_END,
    DEALLOC,
};

enum class InstrArgType {
    REG,
    REG_OFF,
    LIT,
    STACK_LOC,
    PARAM,
    NAN_INSTR_ARG,
    MARK,
    SCOPE_REF,
};

enum class Reg {
    REG_0,
    REG_1,
    REG_2,
};

struct RegOff {
    Reg reg;
    int off;
};

class Scope;
struct StackLoc {
    Scope* scope;
    int stackOff;
};

struct ParamIndex {
    int p;
};

struct NanInstrArg {

};

struct Mark {
    std::string_view m;
};

struct Lit {
    int val;
};

struct InstrArg {
    InstrArgType tp;

    union {
        Mark mark;
        StackLoc st; 
        Reg reg;
        RegOff regOff;
        Lit lit;
        ParamIndex param;
        NanInstrArg nan;
        Scope* scope;
    } data;

    const Mark& as_mark() const {
        ex_assert(tp == InstrArgType::MARK);
        return data.mark;
    }

    const StackLoc& as_stack_loc() const {
        ex_assert(tp == InstrArgType::STACK_LOC);
        return data.st;
    }

    const Reg& as_reg() const {
        ex_assert(tp == InstrArgType::REG);
        return data.reg;
    }

    const Scope* as_scope() const {
        ex_assert(tp == InstrArgType::SCOPE_REF);
        return data.scope;
    }

    const ParamIndex& as_param_index() const {
        ex_assert(tp == InstrArgType::PARAM);
        return data.param;
    }

    const Lit& as_lit() const {
        ex_assert(tp == InstrArgType::LIT);
        return data.lit;
    }
};

constexpr auto NanInstrarg = InstrArg{ .tp = InstrArgType::NAN_INSTR_ARG, .data = { .nan = {} }};


struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
    InstrArg arg3;
    int size;
};


#endif