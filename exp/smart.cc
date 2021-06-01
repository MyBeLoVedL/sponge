#include "common.hh"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

void alloc_name() {
    string name("Alle");
    auto buf = std::vector<int>(100);
    // auto bufp = new vector<int>(200);
    auto sp = make_unique<int[]>(100000);
    sp[2] = 23;
    // *sp = 23;
    // auto s = new int(10);
    // *s = 10;
    // *sp = 12;
    // bufp->push_back(20);
    cout << name << "\n";
}

int main() {
    alloc_name();
    i32 age = 10;
    string a("Vega");
    cout << a << age << "\n";
    return 0;
}