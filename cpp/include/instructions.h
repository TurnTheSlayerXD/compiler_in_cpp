
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <help.h>

constexpr int PTR_SIZE = 8;
constexpr int FUN_PARAMS_STACK_OFF = 40;



enum class InstrType {
    //has no args
    STACKALLOC,
    // has two args
    MOV,

    PUT_PARAM,

    FUN,
    MARK,
};

enum class InstrArgType {
    REG,
    REG_OFF,
    LIT,
    STACK_LOC,
    PARAM,
    NAN_INSTR_ARG,
    MARK,
};

struct Reg {
    int id;
};

struct RegOff {
    Reg reg;
    int off;
};

struct StackLoc {
    std::string_view scopeId;
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
    } data;
};

constexpr auto NanInstrArgType = InstrArg{ .tp = InstrArgType::NAN_INSTR_ARG, .data = { .nan = {} }};


struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
    InstrArg arg3;
    int size;
};


std::string_view to_string(const InstrType &tp) {
    using enum InstrType;
    switch (tp) {
        case STACKALLOC : return "STACKALLOC";
        case MOV : return "MOV";
        case PUT_PARAM : return "PUT_PARAM";
        case FUN : return "FUN";
        case MARK : return "MARK";
        default: assert(false && "UNREACHABLE"); return "";
    }
}


std::string to_string(const InstrArg& arg) {
    using enum InstrArgType;
    switch (arg.tp) {
case REG: 
    return str_fmt("REG[%d]", arg.data.reg.id);
case REG_OFF: 
    return str_fmt("%d(REG[%d])", arg.data.regOff.off, arg.data.regOff.reg.id);
case LIT: 
    return str_fmt("LIT[%d]", arg.data.lit.val);
case STACK_LOC: 
return str_fmt("STACK[%d, %.*s]", arg.data.st.stackOff, arg.data.st.scopeId.size(), arg.data.st.scopeId.data());
case PARAM: 
    return str_fmt("PARAM[%d]", arg.data.param.p);
case NAN_INSTR_ARG: 
    return "NAN";
case MARK: 
    return str_fmt("MARK[%.*s]", arg.data.mark.m.size(), arg.data.mark.m.data());
default: assert(false && "UNREACHABLE"); return "";
    }
}



std::string to_string(const Instr& i) {
    using enum InstrType;
    
    switch (i.tp) {

case STACKALLOC : 
    return std::string("STACKALLOC") + " " + to_string(i.arg1);
case MOV :
    return std::string("MOV") + " " + to_string(i.arg1) + " " + to_string(i.arg2);
case PUT_PARAM :
    return std::string("PUT_PARAM") + " " + to_string(i.arg1);
case FUN :
    return std::string("FUN") + " " + to_string(i.arg1);
case MARK :
    return std::string("MARK") + " " + to_string(i.arg1);
default: assert(false && "UNREACHABLE"); return "";

    }
}

#endif