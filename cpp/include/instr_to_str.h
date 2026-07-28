#ifndef INSTR_TO_STR_H
#define INSTR_TO_STR_H

#include <context.h>

std::string_view to_string(const InstrType &tp) {
    using enum InstrType;
    switch (tp) {
        case STACKALLOC : return "STACKALLOC";
        case MOV : return "MOV";
        case GET_PARAM : return "GET_PARAM";
        case FUN : return "FUN";
        case MARK : return "MARK";
        default: assert(false && "UNREACHABLE"); return "";
    }
}


std::string to_string(const InstrArg& arg) {
    using enum InstrArgType;
    switch (arg.tp) {
case REG: {
    return str_fmt("REG[%d]", arg.data.reg);
}
case REG_OFF: {
    return str_fmt("%d(REG[%d])", arg.data.regOff.off, arg.data.regOff.reg);
}
case LIT: {
    return str_fmt("LIT[%d]", arg.data.lit.val);
}
case STACK_LOC: {
    const auto& scopeName = arg.data.st.scope->id;
    return str_fmt("STACK[%d, %.*s]", arg.data.st.stackOff, scopeName.size(), scopeName.data());
}
case PARAM: {
    return str_fmt("PARAM[%d]", arg.data.param.p);
}
case NAN_INSTR_ARG: {
    return "NAN";
}
case MARK: {
    return str_fmt("MARK[%.*s]", arg.data.mark.m.size(), arg.data.mark.m.data());
}
default: {assert(false && "UNREACHABLE"); return "";}
    }
}



std::string to_string(const Instr& i) {

    using enum InstrType;
    switch (i.tp) {

case SCOPE_START:
    return str_fmt("#START %.*s", SV_ARG(i.arg1.as_scope()->id));
case SCOPE_END:
    return str_fmt("#END %.*s", SV_ARG(i.arg1.as_scope()->id));

case STACKALLOC : 
    return std::string("STACKALLOC") + " " + to_string(i.arg1);
case MOV :
    return std::string("MOV") + " " + to_string(i.arg1) + " " + to_string(i.arg2);
case GET_PARAM :
    return std::string("GET_PARAM") + " " + to_string(i.arg1) + " " + to_string(i.arg2);
case FUN :
    return std::string("FUN") + " " + to_string(i.arg1);
case MARK :

    return std::string("MARK") + " " + to_string(i.arg1);
default: ex_assert(false && "UNREACHABLE"); return "";

    }
}


std::string ctx_instr_to_string(const Context& ctx) {
    std::string res; 
    for (const auto& i: ctx._instrs) {
        res += to_string(i);
        res += "\n";
    }
    return res;
}


#endif