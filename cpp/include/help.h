
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

#endif
