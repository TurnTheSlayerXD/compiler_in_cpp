
#ifndef HELP_H
#define HELP_H

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

#include <memory>

template<typename ... Args>
std::string string_format( const char* format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format, args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format, args ... );
    return std::string(buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

#endif
