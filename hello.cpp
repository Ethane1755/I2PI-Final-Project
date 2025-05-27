#include <iostream>
using namespace std;

class Base {
    public:
        ~Base(){cout << "Base1\n";}
        void print() {cout << "Base2\n";}
};

class Derive : public Base {
    public:
        ~Derive(){cout << "Derive1\n";}
        void print() {cout << "Derive2\n";}
};

int main () {
    Base *ptr = new Derive();
    ptr -> print();
    delete ptr;
    return 0;
}