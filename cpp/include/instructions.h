
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

constexpr int PTR_SIZE = 8;
constexpr int FUN_PARAMS_STACK_OFF = 40;

struct StackLoc {
    int scopeId;
    int stackOff;
    std::string_view nm;
};

struct StackLocOff {
    int scopeId;
    int stackOff;
    int off;
    std::string_view nm;
};


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
    STACK_LOC_OFF,

    PARAM,
    NAN_INSTR,

    MARK,
};

struct Reg {
    int id;
};

struct RegOff {
    Reg reg;
    int off;
};

struct ParamIndex {
    int p;
};

struct NanInstrArg {

};

struct Mark {
    std::string_view m;
};

struct InstrArg {
    InstrArgType tp;

    union {
        Mark mark;

        StackLoc st;
        StackLocOff stOff;

        Reg reg;
        RegOff regOff;
        
        ParamIndex paramIndex;
        NanInstrArg nan;
    } data;
};

constexpr auto NanInstrArgType = InstrArg{ .tp = InstrArgType::NAN_INSTR, .data = { .nan = {} }};


struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
    InstrArg arg3;
    int size;
};

#endif