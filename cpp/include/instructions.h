enum class InstrType {

    MOV,
};

enum class InstrArgType {
    REG,
    REG_OFF,
    LIT,
};

enum class Register {
};



struct InstrArg {
    InstrArgType tp;

    union {
        
        struct {
            Register regId;
            RegisterPart part;  
        } reg;
        
        struct {
            Register regId;
            std::string offset;
        } mem;

    } data;
};



struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
    InstrArg arg3;
};
