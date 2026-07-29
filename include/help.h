
#ifndef HELP_H
#define HELP_H

#include <memory>

#define SV_ARG(sv) static_cast<int>((sv).size()), (sv).data()

template <class T>
struct Destruct {
    T _cbk;
    Destruct(T cbk): _cbk{cbk} { }
    ~Destruct() {
        _cbk();
    }
};

template <class T>
struct Res {
    bool is_err;
    T value;
};


template<typename ... Args>
std::string str_fmt(const char* format, Args ...args )
{   
    (void) format;
    int size_s = std::snprintf(nullptr, 0, format, args... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[size + 1] );
    std::snprintf(buf.get(), size, format, args... );
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

void ex_assert(bool tp, const std::string descr="") {
    if (!tp) {
        throw std::runtime_error(descr);
    }
}

#endif
