enum class InstrType {

};

enum class InstrArgType {

};

struct InstrArg {
    InstrArgType tp;
};

struct Instr {
    InstrType tp;
    InstrArg arg1;
    InstrArg arg2;
};
