#include <iostream>

class Base {
    virtual void fuu() = 0;
};

class Derive : public Base{

    void fuu() override {

    }
};

int main() {

    Derive d;
    Base *b = &d;

    std::cout << "b typeid" << ": " << typeid(b).name() << std::endl;
    std::cout << "b typeid" << ": " << typeid(Derive).name() << std::endl;

    if (typeid(*b) == typeid(Derive)) {
        std::cout << "Right match" << std::endl;
    }
    else {
        std::cout << "False match" << "\t" << typeid(Derive*).name() << std::endl;
    }

    
}