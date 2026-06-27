
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

constexpr int PTR_SIZE = 8;
constexpr int FUN_PARAMS_STACK_OFF = 40;

struct StackP {
    int off;
};

enum class InstrType {
    STACKALLOC,
    MOV,
    MARK,
};

enum class InstrArgType {
    REG,
    REG_OFF,
    LIT,
};

enum class Register {
    STACK_REG,
    P_REG_4,
    P_REG_3,
    P_REG_2,
    P_REG_1,
};


struct InstrArg {
    InstrArgType tp;

    union {
        struct {
            Register regId;
        } reg;
        
        struct {
            Register regId;
            size_t off;
        } mem;
        
    } data;
};


struct RegisterWithOffset {
    Register regId;
    size_t off;
};

RegisterWithOffset reg_off(Register regId, size_t off) {
    return {.regId = regId, .off = off};
}

struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
    InstrArg arg3;
};

#endif